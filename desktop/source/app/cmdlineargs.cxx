/*************************************************************************
 *
 *  $RCSfile: cmdlineargs.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 14:39:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DESKTOP_COMMANDLINEARGS_HXX_
#include <cmdlineargs.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _RTL_URI_HXX_
#include <rtl/uri.hxx>
#endif

namespace desktop
{

static CommandLineArgs::BoolParam aModuleGroupDefinition[] =
{
    CommandLineArgs::CMD_BOOLPARAM_WRITER,
    CommandLineArgs::CMD_BOOLPARAM_CALC,
    CommandLineArgs::CMD_BOOLPARAM_DRAW,
    CommandLineArgs::CMD_BOOLPARAM_IMPRESS,
    CommandLineArgs::CMD_BOOLPARAM_GLOBAL,
    CommandLineArgs::CMD_BOOLPARAM_MATH,
    CommandLineArgs::CMD_BOOLPARAM_WEB,
    CommandLineArgs::CMD_BOOLPARAM_BASE
};

CommandLineArgs::GroupDefinition CommandLineArgs::m_pGroupDefinitions[ CommandLineArgs::CMD_GRPID_COUNT ] =
{
    { 8, aModuleGroupDefinition }
};

CommandLineArgs::CommandLineArgs()
{
    ResetParamValues();
}

// intialize class with command line parameters from process environment
CommandLineArgs::CommandLineArgs( const ::vos::OExtCommandLine& aExtCmdLine )
{
    ResetParamValues();
    ParseCommandLine_Impl( aExtCmdLine );
}

// intialize class with command line parameters from interprocess communication (officeipcthread)
CommandLineArgs::CommandLineArgs( const ::rtl::OUString& aCmdLineArgs )
{
    ResetParamValues();
    ParseCommandLine_String( aCmdLineArgs );
}

// ----------------------------------------------------------------------------

void CommandLineArgs::ParseCommandLine_Impl( const ::vos::OExtCommandLine& aExtCmdLine )
{
    ::vos::OExtCommandLine aCmdLine;

    sal_uInt32      nCount = aCmdLine.getCommandArgCount();
    ::rtl::OUString aDummy;
    String          aArguments;

    // Extract cmdline parameters and concat them to the cmdline string format
    for( sal_uInt32 i=0; i < nCount; i++ )
    {
        aCmdLine.getCommandArg( i, aDummy );
        aArguments += String( aDummy );
        aArguments += '|';
    }

    // Parse string as a cmdline string
    ParseCommandLine_String( ::rtl::OUString( aArguments ));
}

void CommandLineArgs::AddStringListParam_Impl( StringParam eParam, const rtl::OUString& aParam )
{
    OSL_ASSERT( eParam >= 0 && eParam < CMD_STRINGPARAM_COUNT );
    if ( m_aStrParams[eParam].getLength() )
        m_aStrParams[eParam] += ::rtl::OUString::valueOf( (sal_Unicode)APPEVENT_PARAM_DELIMITER );
    m_aStrParams[eParam] += aParam;
    m_aStrSetParams[eParam] = sal_True;
}

void CommandLineArgs::SetBoolParam_Impl( BoolParam eParam, sal_Bool bValue )
{
    OSL_ASSERT( eParam >= 0 && eParam < CMD_BOOLPARAM_COUNT );
    m_aBoolParams[eParam] = bValue;
}

void CommandLineArgs::ParseCommandLine_String( const ::rtl::OUString& aCmdLineString )
{
    // parse command line arguments
    sal_Bool    bPrintEvent     = sal_False;
    sal_Bool    bOpenEvent      = sal_True;
    sal_Bool    bViewEvent      = sal_False;
    sal_Bool    bStartEvent     = sal_False;
    sal_Bool    bPrintToEvent   = sal_False;
    sal_Bool    bPrinterName    = sal_False;
    sal_Bool    bForceOpenEvent = sal_False;
    sal_Bool    bForceNewEvent  = sal_False;
    sal_Bool    bDisplaySpec    = sal_False;

    m_bEmpty = (aCmdLineString.getLength()<1);

    sal_Int32 nIndex = 0;
    do
    {
        ::rtl::OUString aArg    = aCmdLineString.getToken( 0, '|', nIndex );
        String          aArgStr = aArg;

        if ( aArg.getLength() > 0 )
        {
            if ( !InterpretCommandLineParameter( aArg ))
            {
                if ( aArgStr.GetChar(0) == '-' )
                {
                    // handle this argument as an option
                    if ( aArgStr.EqualsIgnoreCaseAscii( "-n" ))
                    {
                        // force new documents based on the following documents
                        bForceNewEvent  = sal_True;
                        bOpenEvent      = sal_False;
                        bForceOpenEvent = sal_False;
                        bPrintToEvent   = sal_False;
                        bPrintEvent     = sal_False;
                        bViewEvent      = sal_False;
                        bStartEvent     = sal_False;
                        bDisplaySpec    = sal_False;
                     }
                    else if ( aArgStr.EqualsIgnoreCaseAscii( "-o" ))
                    {
                        // force open documents regards if they are templates or not
                        bForceOpenEvent = sal_True;
                        bOpenEvent      = sal_False;
                        bForceNewEvent  = sal_False;
                        bPrintToEvent   = sal_False;
                        bPrintEvent     = sal_False;
                        bViewEvent      = sal_False;
                        bStartEvent     = sal_False;
                        bDisplaySpec    = sal_False;
                     }
                    else if ( aArgStr.EqualsIgnoreCaseAscii( "-pt" ))
                    {
                        // Print to special printer
                        bPrintToEvent   = sal_True;
                        bPrinterName    = sal_True;
                        bPrintEvent     = sal_False;
                        bOpenEvent      = sal_False;
                        bForceNewEvent  = sal_False;
                        bViewEvent      = sal_False;
                        bStartEvent     = sal_False;
                        bDisplaySpec    = sal_False;
                         bForceOpenEvent = sal_False;
                    }
                    else if ( aArgStr.EqualsIgnoreCaseAscii( "-p" ))
                    {
                        // Print to default printer
                        bPrintEvent     = sal_True;
                        bPrintToEvent   = sal_False;
                        bOpenEvent      = sal_False;
                        bForceNewEvent  = sal_False;
                        bForceOpenEvent = sal_False;
                        bViewEvent      = sal_False;
                        bStartEvent     = sal_False;
                        bDisplaySpec    = sal_False;
                 }
                    else if ( aArgStr.EqualsIgnoreCaseAscii( "-view" ))
                    {
                        // open in viewmode
                        bOpenEvent      = sal_False;
                        bPrintEvent     = sal_False;
                        bPrintToEvent   = sal_False;
                        bForceNewEvent  = sal_False;
                        bForceOpenEvent = sal_False;
                        bViewEvent      = sal_True;
                        bStartEvent     = sal_False;
                        bDisplaySpec    = sal_False;
                 }
                    else if ( aArgStr.EqualsIgnoreCaseAscii( "-show" ))
                    {
                            // open in viewmode
                            bOpenEvent      = sal_False;
                            bViewEvent      = sal_False;
                            bStartEvent     = sal_True;
                            bPrintEvent     = sal_False;
                            bPrintToEvent   = sal_False;
                            bForceNewEvent  = sal_False;
                            bForceOpenEvent = sal_False;
                            bDisplaySpec    = sal_False;
                    }
                    else if ( aArgStr.EqualsIgnoreCaseAscii( "-display" ))
                    {
                            // open in viewmode
                            bOpenEvent      = sal_False;
                               bPrintEvent     = sal_False;
                            bForceOpenEvent = sal_False;
                            bPrintToEvent   = sal_False;
                            bForceNewEvent  = sal_False;
                            bViewEvent      = sal_False;
                            bStartEvent     = sal_False;
                            bDisplaySpec    = sal_True;
                    }

                }
                else
                {
                    if ( bPrinterName && bPrintToEvent )
                    {
                        // first argument after "-pt" this must be the printer name
                        AddStringListParam_Impl( CMD_STRINGPARAM_PRINTERNAME, aArgStr );
                        bPrinterName = sal_False;
                    }
                    else
                    {
                        // handle this argument as a filename
                        if ( bOpenEvent )
                            AddStringListParam_Impl( CMD_STRINGPARAM_OPENLIST, aArgStr );
                        else if ( bViewEvent )
                            AddStringListParam_Impl( CMD_STRINGPARAM_VIEWLIST, aArgStr );
                        else if ( bStartEvent )
                            AddStringListParam_Impl( CMD_STRINGPARAM_STARTLIST, aArgStr );
                        else if ( bPrintEvent )
                            AddStringListParam_Impl( CMD_STRINGPARAM_PRINTLIST, aArgStr );
                        else if ( bPrintToEvent )
                            AddStringListParam_Impl( CMD_STRINGPARAM_PRINTTOLIST, aArgStr );
                        else if ( bForceNewEvent )
                            AddStringListParam_Impl( CMD_STRINGPARAM_FORCENEWLIST, aArgStr );
                        else if ( bForceOpenEvent )
                            AddStringListParam_Impl( CMD_STRINGPARAM_FORCEOPENLIST, aArgStr );
                        else if ( bDisplaySpec ){
                            AddStringListParam_Impl( CMD_STRINGPARAM_DISPLAY, aArgStr );
                            bDisplaySpec = sal_False; // only one display, not a lsit
                            bOpenEvent = sal_True;    // set back to standard
                        }
                    }
                }
            }
        }
    }
    while ( nIndex >= 0 );
}

sal_Bool CommandLineArgs::InterpretCommandLineParameter( const ::rtl::OUString& aArg )
{
    String aArgStr( aArg );

    if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-minimized" )) == sal_True )
    {
        SetBoolParam_Impl( CMD_BOOLPARAM_MINIMIZED, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-invisible" )) == sal_True )
    {
        SetBoolParam_Impl( CMD_BOOLPARAM_INVISIBLE, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-norestore" )) == sal_True )
    {
        SetBoolParam_Impl( CMD_BOOLPARAM_NORESTORE, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-nodefault" )) == sal_True )
    {
        SetBoolParam_Impl( CMD_BOOLPARAM_NODEFAULT, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-bean" )) == sal_True )
    {
        SetBoolParam_Impl( CMD_BOOLPARAM_BEAN, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-plugin" )) == sal_True )
    {
        SetBoolParam_Impl( CMD_BOOLPARAM_PLUGIN, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-server" )) == sal_True )
    {
        SetBoolParam_Impl( CMD_BOOLPARAM_SERVER, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-headless" )) == sal_True )
    {
        // Headless means also invisibile, so set this parameter to true!
        SetBoolParam_Impl( CMD_BOOLPARAM_HEADLESS, sal_True );
        SetBoolParam_Impl( CMD_BOOLPARAM_INVISIBLE, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-quickstart" )) == sal_True )
    {
        SetBoolParam_Impl( CMD_BOOLPARAM_QUICKSTART, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-terminate_after_init" )) == sal_True )
    {
        SetBoolParam_Impl( CMD_BOOLPARAM_TERMINATEAFTERINIT, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-nologo" )) == sal_True )
    {
        SetBoolParam_Impl( CMD_BOOLPARAM_NOLOGO, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-nolockcheck" )) == sal_True )
    {
        SetBoolParam_Impl( CMD_BOOLPARAM_NOLOCKCHECK, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-help" ))
        || aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-h" ))
        || aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-?" )))
    {
        SetBoolParam_Impl( CMD_BOOLPARAM_HELP, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-helpwriter" )) == sal_True )
    {
        SetBoolParam_Impl( CMD_BOOLPARAM_HELPWRITER, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-helpcalc" )) == sal_True )
    {
        SetBoolParam_Impl( CMD_BOOLPARAM_HELPCALC, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-helpdraw" )) == sal_True )
    {
        SetBoolParam_Impl( CMD_BOOLPARAM_HELPDRAW, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-helpimpress" )) == sal_True )
    {
        SetBoolParam_Impl( CMD_BOOLPARAM_HELPIMPRESS, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-helpbase" )) == sal_True )
    {
        SetBoolParam_Impl( CMD_BOOLPARAM_HELPBASE, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-helpbasic" )) == sal_True )
    {
        SetBoolParam_Impl( CMD_BOOLPARAM_HELPBASIC, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-helpmath" )) == sal_True )
    {
        SetBoolParam_Impl( CMD_BOOLPARAM_HELPMATH, sal_True );
        return sal_True;
    }
    else if ( aArgStr.Copy(0, 8).EqualsIgnoreCaseAscii( "-accept=" ))
    {
        AddStringListParam_Impl( CMD_STRINGPARAM_ACCEPT, aArgStr.Copy( 8 ) );
        return sal_True;
    }
    else if ( aArgStr.Copy(0, 10).EqualsIgnoreCaseAscii( "-unaccept=" ))
    {
        AddStringListParam_Impl( CMD_STRINGPARAM_UNACCEPT, aArgStr.Copy( 10 ) );
        return sal_True;
    }
    else if ( aArgStr.CompareIgnoreCaseToAscii( "-portal," ,
                                                RTL_CONSTASCII_LENGTH( "-portal," )) == COMPARE_EQUAL )
    {
        AddStringListParam_Impl( CMD_STRINGPARAM_PORTAL, aArgStr.Copy( RTL_CONSTASCII_LENGTH( "-portal," )) );
        return sal_True;
    }
    else if ( aArgStr.Copy( 0, 7 ).EqualsIgnoreCaseAscii( "-userid" ))
    {
        if ( aArgStr.Len() > 8 )
        {
            rtl::OUString aUserDir = aArgStr;
            AddStringListParam_Impl(
                CMD_STRINGPARAM_USERDIR,
                ::rtl::Uri::decode( aUserDir.copy( 8 ),
                                    rtl_UriDecodeWithCharset,
                                    RTL_TEXTENCODING_UTF8 ) );
        }
        return sal_True;
    }
    else if ( aArgStr.Copy( 0, 15).EqualsIgnoreCaseAscii( "-clientdisplay=" ))
    {
        AddStringListParam_Impl( CMD_STRINGPARAM_CLIENTDISPLAY, aArgStr.Copy( 15 ) );
        return sal_True;
    }
    else if ( aArgStr.Copy(0, 9).EqualsIgnoreCaseAscii( "-version=" ))
    {
        AddStringListParam_Impl( CMD_STRINGPARAM_VERSION, aArgStr.Copy( 15 ) );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-writer" )) == sal_True )
    {
        sal_Bool bAlreadySet = CheckGroupMembers( CMD_GRPID_MODULE, CMD_BOOLPARAM_WRITER );
        if ( !bAlreadySet )
            SetBoolParam_Impl( CMD_BOOLPARAM_WRITER, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-calc" )) == sal_True )
    {
        sal_Bool bAlreadySet = CheckGroupMembers( CMD_GRPID_MODULE, CMD_BOOLPARAM_CALC );
        if ( !bAlreadySet )
            SetBoolParam_Impl( CMD_BOOLPARAM_CALC, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-draw" )) == sal_True )
    {
        sal_Bool bAlreadySet = CheckGroupMembers( CMD_GRPID_MODULE, CMD_BOOLPARAM_DRAW );
        if ( !bAlreadySet )
            SetBoolParam_Impl( CMD_BOOLPARAM_DRAW, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-impress" )) == sal_True )
    {
        sal_Bool bAlreadySet = CheckGroupMembers( CMD_GRPID_MODULE, CMD_BOOLPARAM_IMPRESS );
        if ( !bAlreadySet )
            SetBoolParam_Impl( CMD_BOOLPARAM_IMPRESS, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-base" )) == sal_True )
    {
        sal_Bool bAlreadySet = CheckGroupMembers( CMD_GRPID_MODULE, CMD_BOOLPARAM_BASE );
        if ( !bAlreadySet )
            SetBoolParam_Impl( CMD_BOOLPARAM_BASE, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-global" )) == sal_True )
    {
        sal_Bool bAlreadySet = CheckGroupMembers( CMD_GRPID_MODULE, CMD_BOOLPARAM_GLOBAL );
        if ( !bAlreadySet )
            SetBoolParam_Impl( CMD_BOOLPARAM_GLOBAL, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-math" )) == sal_True )
    {
        sal_Bool bAlreadySet = CheckGroupMembers( CMD_GRPID_MODULE, CMD_BOOLPARAM_MATH );
        if ( !bAlreadySet )
            SetBoolParam_Impl( CMD_BOOLPARAM_MATH, sal_True );
        return sal_True;
    }
    else if ( aArg.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "-web" )) == sal_True )
    {
        sal_Bool bAlreadySet = CheckGroupMembers( CMD_GRPID_MODULE, CMD_BOOLPARAM_WEB );
        if ( !bAlreadySet )
            SetBoolParam_Impl( CMD_BOOLPARAM_WEB, sal_True );
        return sal_True;
    }

    return sal_False;
}

sal_Bool CommandLineArgs::CheckGroupMembers( GroupParamId nGroupId, BoolParam nExcludeMember ) const
{
    // Check if at least one bool param out of a group is set. An exclude member can be provided.
    for ( int i = 0; i < m_pGroupDefinitions[nGroupId].nCount; i++ )
    {
        BoolParam nParam = m_pGroupDefinitions[nGroupId].pGroupMembers[i];
        if ( nParam != nExcludeMember && m_aBoolParams[nParam] )
            return sal_True;
    }

    return sal_False;
}

void CommandLineArgs::ResetParamValues()
{
    int i;

    for ( i = 0; i < CMD_BOOLPARAM_COUNT; i++ )
        m_aBoolParams[i] = sal_False;
    for ( i = 0; i < CMD_STRINGPARAM_COUNT; i++ )
        m_aStrSetParams[i] = sal_False;
    m_bEmpty = sal_True;
}

sal_Bool CommandLineArgs::GetBoolParam( BoolParam eParam ) const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );

    OSL_ASSERT( ( eParam >= 0 && eParam < CMD_BOOLPARAM_COUNT ) );
    return m_aBoolParams[eParam];
}

void CommandLineArgs::SetBoolParam( BoolParam eParam, sal_Bool bNewValue )
{
    osl::MutexGuard  aMutexGuard( m_aMutex );

    OSL_ASSERT( ( eParam >= 0 && eParam < CMD_BOOLPARAM_COUNT ) );
    m_aBoolParams[eParam] = bNewValue;
}

const rtl::OUString& CommandLineArgs::GetStringParam( BoolParam eParam ) const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );

    OSL_ASSERT( ( eParam >= 0 && eParam < CMD_STRINGPARAM_COUNT ) );
    return m_aStrParams[eParam];
}

void CommandLineArgs::SetStringParam( BoolParam eParam, const rtl::OUString& aNewValue )
{
    osl::MutexGuard  aMutexGuard( m_aMutex );

    OSL_ASSERT( ( eParam >= 0 && eParam < CMD_STRINGPARAM_COUNT ) );
    m_aStrParams[eParam] = aNewValue;
}

sal_Bool CommandLineArgs::IsMinimized() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_MINIMIZED ];
}

sal_Bool CommandLineArgs::IsInvisible() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_INVISIBLE ];
}

sal_Bool CommandLineArgs::IsNoRestore() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_NORESTORE ];
}

sal_Bool CommandLineArgs::IsNoDefault() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_NODEFAULT ];
}

sal_Bool CommandLineArgs::IsBean() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_BEAN ];
}

sal_Bool CommandLineArgs::IsPlugin() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_PLUGIN ];
}

sal_Bool CommandLineArgs::IsServer() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_SERVER ];
}

sal_Bool CommandLineArgs::IsHeadless() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_HEADLESS ];
}

sal_Bool CommandLineArgs::IsQuickstart() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_QUICKSTART ];
}

sal_Bool CommandLineArgs::IsTerminateAfterInit() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_TERMINATEAFTERINIT ];
}

sal_Bool CommandLineArgs::IsNoLogo() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_NOLOGO ];
}

sal_Bool CommandLineArgs::IsNoLockcheck() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_NOLOCKCHECK ];
}

sal_Bool CommandLineArgs::IsHelp() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_HELP ];
}
sal_Bool CommandLineArgs::IsHelpWriter() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_HELPWRITER ];
}

sal_Bool CommandLineArgs::IsHelpCalc() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_HELPCALC ];
}

sal_Bool CommandLineArgs::IsHelpDraw() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_HELPDRAW ];
}

sal_Bool CommandLineArgs::IsHelpImpress() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_HELPIMPRESS ];
}

sal_Bool CommandLineArgs::IsHelpBase() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_HELPBASE ];
}
sal_Bool CommandLineArgs::IsHelpMath() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_HELPMATH ];
}
sal_Bool CommandLineArgs::IsHelpBasic() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_HELPBASIC ];
}

sal_Bool CommandLineArgs::IsWriter() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_WRITER ];
}

sal_Bool CommandLineArgs::IsCalc() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_CALC ];
}

sal_Bool CommandLineArgs::IsDraw() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_DRAW ];
}

sal_Bool CommandLineArgs::IsImpress() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_IMPRESS ];
}

sal_Bool CommandLineArgs::IsBase() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_BASE ];
}

sal_Bool CommandLineArgs::IsGlobal() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_GLOBAL ];
}

sal_Bool CommandLineArgs::IsMath() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_MATH ];
}

sal_Bool CommandLineArgs::IsWeb() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_aBoolParams[ CMD_BOOLPARAM_WEB ];
}

sal_Bool CommandLineArgs::HasModuleParam() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return CheckGroupMembers( CMD_GRPID_MODULE, CMD_BOOLPARAM_COUNT );
}

sal_Bool CommandLineArgs::GetPortalConnectString( ::rtl::OUString& rPara ) const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    rPara = m_aStrParams[ CMD_STRINGPARAM_PORTAL ];
    return m_aStrSetParams[ CMD_STRINGPARAM_PORTAL ];
}

sal_Bool CommandLineArgs::GetAcceptString( ::rtl::OUString& rPara ) const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    rPara = m_aStrParams[ CMD_STRINGPARAM_ACCEPT ];
    return m_aStrSetParams[ CMD_STRINGPARAM_ACCEPT ];
}

sal_Bool CommandLineArgs::GetUnAcceptString( ::rtl::OUString& rPara ) const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    rPara = m_aStrParams[ CMD_STRINGPARAM_UNACCEPT ];
    return m_aStrSetParams[ CMD_STRINGPARAM_UNACCEPT ];
}

sal_Bool CommandLineArgs::GetUserDir( ::rtl::OUString& rPara) const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    rPara = m_aStrParams[ CMD_STRINGPARAM_USERDIR ];
    return m_aStrSetParams[ CMD_STRINGPARAM_USERDIR ];
}

sal_Bool CommandLineArgs::GetClientDisplay( ::rtl::OUString& rPara) const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    rPara = m_aStrParams[ CMD_STRINGPARAM_CLIENTDISPLAY ];
    return m_aStrSetParams[ CMD_STRINGPARAM_CLIENTDISPLAY ];
}

sal_Bool CommandLineArgs::GetOpenList( ::rtl::OUString& rPara) const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    rPara = m_aStrParams[ CMD_STRINGPARAM_OPENLIST ];
    return m_aStrSetParams[ CMD_STRINGPARAM_OPENLIST ];
}

sal_Bool CommandLineArgs::GetViewList( ::rtl::OUString& rPara) const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    rPara = m_aStrParams[ CMD_STRINGPARAM_VIEWLIST ];
    return m_aStrSetParams[ CMD_STRINGPARAM_VIEWLIST ];
}

sal_Bool CommandLineArgs::GetStartList( ::rtl::OUString& rPara) const
{
      osl::MutexGuard  aMutexGuard( m_aMutex );
      rPara = m_aStrParams[ CMD_STRINGPARAM_STARTLIST ];
      return m_aStrSetParams[ CMD_STRINGPARAM_STARTLIST ];
}

sal_Bool CommandLineArgs::GetForceOpenList( ::rtl::OUString& rPara) const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    rPara = m_aStrParams[ CMD_STRINGPARAM_FORCEOPENLIST ];
    return m_aStrSetParams[ CMD_STRINGPARAM_FORCEOPENLIST ];
}

sal_Bool CommandLineArgs::GetForceNewList( ::rtl::OUString& rPara) const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    rPara = m_aStrParams[ CMD_STRINGPARAM_FORCENEWLIST ];
    return m_aStrSetParams[ CMD_STRINGPARAM_FORCENEWLIST ];
}

sal_Bool CommandLineArgs::GetPrintList( ::rtl::OUString& rPara) const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    rPara = m_aStrParams[ CMD_STRINGPARAM_PRINTLIST ];
    return m_aStrSetParams[ CMD_STRINGPARAM_PRINTLIST ];
}

sal_Bool CommandLineArgs::GetVersionString( ::rtl::OUString& rPara) const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    rPara = m_aStrParams[ CMD_STRINGPARAM_VERSION ];
    return m_aStrSetParams[ CMD_STRINGPARAM_VERSION ];
}

sal_Bool CommandLineArgs::GetPrintToList( ::rtl::OUString& rPara ) const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    rPara = m_aStrParams[ CMD_STRINGPARAM_PRINTTOLIST ];
    return m_aStrSetParams[ CMD_STRINGPARAM_PRINTTOLIST ];
}

sal_Bool CommandLineArgs::GetPrinterName( ::rtl::OUString& rPara ) const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    rPara = m_aStrParams[ CMD_STRINGPARAM_PRINTERNAME ];
    return m_aStrSetParams[ CMD_STRINGPARAM_PRINTERNAME ];
}

sal_Bool CommandLineArgs::GetDisplay( ::rtl::OUString& rPara ) const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    rPara = m_aStrParams[ CMD_STRINGPARAM_DISPLAY ];
    return m_aStrSetParams[ CMD_STRINGPARAM_DISPLAY ];
}

sal_Bool CommandLineArgs::IsPrinting() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return( m_aStrParams[ CMD_STRINGPARAM_PRINTLIST ].getLength() > 0 ||
            m_aStrParams[ CMD_STRINGPARAM_PRINTTOLIST ].getLength() > 0 );
}

sal_Bool CommandLineArgs::IsEmpty() const
{
    osl::MutexGuard  aMutexGuard( m_aMutex );
    return m_bEmpty;
}

} // namespace desktop
