/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#if OSL_DEBUG_LEVEL > 1
#include <tools/stream.hxx>
#endif

#include "macro.hxx"
#include <sfx2/request.hxx>
#include <sfx2/msg.hxx>

//====================================================================

SV_DECL_PTRARR_DEL( SfxStatements_Impl, SfxMacroStatement*, 16, 8 )
SV_IMPL_PTRARR( SfxStatements_Impl, SfxMacroStatement* );

//--------------------------------------------------------------------

struct SfxMacro_Impl

/*  [Description]

    Implementation structure for the <SfxMacro> class.
*/

{
    SfxMacroMode        eMode;  /* purpose of the <SfxMacro> instance,
                                   for meaning see enum <SfxMacroMode> */
    SfxStatements_Impl  aList;  /* List of recorded Statements */
};

//====================================================================

SfxMacroStatement::SfxMacroStatement
(
    const SfxShell& /*rShell*/,    // <SfxShell> which excexutes the Request
    const String&   /*rTarget*/,   // Name of the target object for the
                                   // excecution
    sal_Bool        /*bAbsolute*/, // obsolete
    const SfxSlot&  rSlot,         // the <SfxSlot>, which can read the
                                   // statement
    sal_Bool          bRequestDone,  // Was the Request really excecuted
    ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& rArgs
)

/*  [Description]

    This constructor of the SfxMacroStatement class generates a statement in
    which an object is brought up, which is given by 'rShell'. Thus It is
    addressing the always after 'bAbsolute' absolute, for example, as
    '[mydoc.sdc]', or relative, that is 'active document'.

    Depending on the subclass of 'rShell', the following terms:

                          | absolute                 relative
    -----------------------------------------------------------------------
    SfxApplication'       | 'StarCalc'              'Application'
    SfxViewFrame'         | '[mydoc.sdc:1]'         'ActiveWindow'
    SfxViewShell'         | '[mydoc.sdc:1]'         'AvtiveWindow'
    SfxObjectShell'       | '[mydoc.sdc]'           'ActiveDocument'
    Other (Sub-Shells)    | '[mydoc.sdc:1]'         'ActiveWindow'

    Thus 'StarCalc' is a proxy for the name of the application
    (Application::GetAppName()const). In the absolute sense of the selection
    could be addressed as descriptive as in 'cell selection ("A5-D8")'), to do this
    the application programmer must however use the constructor
    <SfxMacroStatement::SfxMacroStatement(const String&,
    const SfxSlot&,sal_Bool,SfxArguments*)>.

    For the so-called object is then, depending on the type of slots,
    expressed as an assignment to one of its properties or calling its methods.

    [Example]

    absolute:
    SCalc3.OpenDocument( "\docs\mydoc.sdd", "StarDraw Presentation", 0, 0 )
    [mydoc.sdd].Activate()
    [mydoc.sdd:1].SwitchToView( 2 )
    [mydoc.sdc:1:1].CellSelection( "A5-D8" ).LineColor = 192357

    relative:
    ActiveWindow.LineColor = 192357

    [Cross-reference]

    <SfxMacroStatement::SfxMacroStatement(const String&,const SfxSlot&,sal_Bool,SfxArguments*)>
    <SfxMacroStatement::SfxMacroStatement(const String&)>
*/

:       nSlotId( rSlot.GetSlotId() ),
    aArgs( rArgs ),
    bDone( bRequestDone ),
    pDummy( 0 )
{
    // Workaround Recording non exported Slots
    if ( !rSlot.pName )
        return;

    aStatement = DEFINE_CONST_UNICODE("Selection");

    // to these object expression  of the Method-/Property-Name and parameters
    GenerateNameAndArgs_Impl( rSlot, bRequestDone, aArgs);
}

//--------------------------------------------------------------------

SfxMacroStatement::SfxMacroStatement
(
    const String&   rTarget,  // The Object which is addressed while playing
    const SfxSlot&  rSlot,    // The <SfxSlot>, which can playback the statement
    sal_Bool        bRequestDone,  // was the Request actually performed
    ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& rArgs
)

/*  [Description]


    [Cross-reference]

    <SfxMacroStatement::SfxMacroStatement(const String&)>
    <SfxMacroStatement::SfxMacroStatement(const SfxShell&,sal_Bool,const SfxSlot&,sal_Bool,SfxArguments*)>
*/

:       nSlotId( rSlot.GetSlotId() ),
    aArgs( rArgs ),
    bDone( bRequestDone ),
    pDummy( 0 )
{
    aStatement = rTarget;
    aStatement += '.';
    GenerateNameAndArgs_Impl( rSlot, bRequestDone, aArgs);
}

//--------------------------------------------------------------------

SfxMacroStatement::SfxMacroStatement
(
    const String&   rStatement  // manually generated statement(s)
)

/*  [Description]

    This constructor creates a SfxMacroStatement instance, its structure is
    determined completely by the application developer. Because the specified
    string may contain several statements separated with CR/LF, which enables
    a extensive in the recording of BASIC-macros, in order to handle special
    cases.

    [Cross-reference]

    <SfxMacroStatement::SfxMacroStatement(const String&,const SfxSlot&,sal_Bool,SfxArguments*)>
    <SfxMacroStatement::SfxMacroStatement(const SfxShell&,sal_Bool,const SfxSlot&,sal_Bool,SfxArguments*)>
*/

:       nSlotId( 0 ),
       aStatement( rStatement ),
    bDone( sal_True ),
    pDummy( 0 )
{
}

//--------------------------------------------------------------------

SfxMacroStatement::SfxMacroStatement
(
    const SfxMacroStatement&    rOrig // Original, from which it will be copied
)

/*  [Description]

    Copy constructor of the sSfxMacroStatement class.
*/

:       nSlotId( rOrig.nSlotId ),
    aStatement( rOrig.aStatement ),
       bDone( rOrig.bDone ),
    pDummy( 0 )
{
    aArgs = rOrig.aArgs;
}

//--------------------------------------------------------------------

SfxMacroStatement::~SfxMacroStatement()

/*  [Description]

    Destructor of the SfxMacroStatement class. Frees the list of the
    current parameters.
*/

{
}

//--------------------------------------------------------------------

void SfxMacroStatement::GenerateNameAndArgs_Impl
(
    const SfxSlot&   rSlot,        // the slot, which can playback the statement
    sal_Bool         bRequestDone, // TRUE=was executed, FALSE=cancelled
    ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >& /*rArgs*/
)

/*  [Description]

    Internal utility method for generating the functions-/property-names as
    well as the parameter. This method is only used if the application
    programmer has not attached his own source to the <SfxRequest>.
*/

{
    if ( aStatement.Len() && aStatement.GetChar( aStatement.Len() - 1 ) != '.'
         && rSlot.pName[0] != '.' )
        aStatement += '.';

    // The name of the slots is the name of the method or properties
    aStatement += String::CreateFromAscii(rSlot.pName);
    if ( rSlot.IsMode(SFX_SLOT_METHOD) )
        aStatement += DEFINE_CONST_UNICODE("( ");
    else
        aStatement += DEFINE_CONST_UNICODE(" = ");

    // Print all required Parameters
    if ( aArgs.getLength() )
        for ( sal_uInt16 nArg = 0; nArg < aArgs.getLength(); ++nArg )
        {
            // represent the parameters textually
            String aArg;
            ::com::sun::star::uno::Any& rValue = aArgs[nArg].Value;
            ::com::sun::star::uno::Type pType = rValue.getValueType();
            if ( pType == ::getBooleanCppuType() )
            {
                sal_Bool bTemp = false;
                rValue >>= bTemp;
                aArg = bTemp ? DEFINE_CONST_UNICODE("TRUE") : DEFINE_CONST_UNICODE("FALSE");
            }
            else if ( pType == ::getCppuType((const sal_Int16*)0) )
            {
                sal_uInt16 nTemp = 0;
                rValue >>= nTemp;
                aArg = String::CreateFromInt32( (sal_Int32) nTemp );
            }
            else if ( pType == ::getCppuType((const sal_Int32*)0) )
            {
                sal_uInt32 nTemp = 0;
                rValue >>= nTemp;
                aArg = String::CreateFromInt32( nTemp );
            }
            else if ( pType == ::getCppuType((const ::rtl::OUString*)0) )
            {
                ::rtl::OUString sTemp;
                rValue >>= sTemp;

                // Quotation marks are doubled
                XubString aRecordable( sTemp );
                sal_uInt16 nPos = 0;
                while ( sal_True )
                {
                    nPos = aRecordable.SearchAndReplace( String('"'), DEFINE_CONST_UNICODE("\"\""), nPos );
                    if ( STRING_NOTFOUND == nPos )
                        break;
                    nPos += 2;
                }

                // non-printable characters are written as chr $(...)
                bool bPrevReplaced = false;
                for ( sal_uInt16 n = 0; n < aRecordable.Len(); ++n )
                {
                    sal_Unicode cChar = aRecordable.GetChar(n);
                    if ( !( cChar>=32 && cChar!=127 ) ) // As replacement for String::IsPrintable()!
                    {
                        XubString aReplacement( DEFINE_CONST_UNICODE("+chr$(") );
                        aReplacement += cChar;

                        if ( bPrevReplaced )
                        {
                            aRecordable.Insert( aReplacement, n - 2 );
                            n = n + aReplacement.Len();
                            aRecordable.SetChar((unsigned short) (n-2), 0x0029);// ')' = 29h
                            aRecordable.Replace( n-1, 2, DEFINE_CONST_UNICODE("+\"") );
                        }
                        else
                        {
                            aReplacement += DEFINE_CONST_UNICODE(")+\"");
                            aRecordable.SetChar(n, 0x0022 );// '"' = 22h
                            aRecordable.Insert( aReplacement, n + 1 );
                            n = n + aReplacement.Len();
                        }
                        bPrevReplaced = true;
                    }
                    else
                        bPrevReplaced = false;

                    // Argument in quotation marks
                    aArg = '"';
                    aArg += aRecordable;
                    aArg += '"';
                }
            }
            else
            {
                OSL_ENSURE(
                    pType == ::getVoidCppuType(), "Unknown Type in recorder!" );
            }

            // Attach the parameter
            aStatement += aArg;
            aStatement += DEFINE_CONST_UNICODE(", ");
        }

    // End statement
    if ( aArgs.getLength() )
        aStatement.Erase( aStatement.Len() - 2, 1 );
    else
        aStatement.Erase( aStatement.Len() - 1, 1 );
    if ( rSlot.IsMode(SFX_SLOT_METHOD) )
        aStatement += ')';

    if ( !bRequestDone )
        // not comment out the "Done ()" marked statements
        aStatement.InsertAscii( "rem ", 0 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
