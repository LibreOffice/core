/*************************************************************************
 *
 *  $RCSfile: process.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: jbu $ $Date: 2001-03-14 16:38:40 $
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


#include <cstdarg>
#include <vector>
#include <rtl/ustring.hxx>

#include "vos/process.hxx"
#include "vos/diagnose.hxx"
#include <osl/file.hxx>

#define MAX_RESOURCES   100
#define MAX_ARGS        100
#define MAX_ENVIROMENTS 100

#ifdef _USE_NAMESPACE
using namespace vos;
#endif


/////////////////////////////////////////////////////////////////////////////
/// Argument

OArgumentList::OArgumentList() :
    n_Args(0),
    m_aVec(0)
{
    // empty
}

OArgumentList::OArgumentList( sal_uInt32 nArgs, const NAMESPACE_RTL(OUString)* aArgument1, ... ) :
    n_Args( nArgs )
{
    m_aVec = new rtl_uString* [nArgs];
    std::va_list pArgs;
    sal_uInt32 i = 0;
    const rtl::OUString* aArgument;

    va_start ( pArgs, aArgument1 );
    aArgument = aArgument1;

    while( true ) {
        m_aVec[i] = aArgument->pData;
        rtl_uString_acquire( m_aVec[i++] );
        if( i < n_Args )
            aArgument = va_arg( pArgs,rtl::OUString* );
        else
            break;
    }
    va_end( pArgs );
}


OArgumentList::OArgumentList( const rtl::OUString aArgumentList[], sal_uInt32 nArgs ) :
    n_Args( nArgs )
{
    m_aVec = new rtl_uString* [n_Args];
    for( sal_uInt32 i = 0; i < n_Args; ++ i )  {
        m_aVec[i] = aArgumentList[i].pData;
        rtl_uString_acquire( m_aVec[i] );
    }
}

OArgumentList::OArgumentList( const OArgumentList& rOther ) : n_Args( rOther.n_Args )
{
    m_aVec = new rtl_uString* [n_Args];

    sal_uInt32 i;
    for ( i = 0; i < n_Args; ++i )
    {
        m_aVec[i] = rOther.m_aVec[i];
        rtl_uString_acquire( m_aVec[i] );
    }
}

OArgumentList& OArgumentList::operator=( const OArgumentList& rOther )
{
    if ( this != &rOther )
    {

        // delete the old one
        sal_uInt32 i;
        for ( i = 0; i < n_Args; ++i )
            rtl_uString_release( m_aVec[i] );

        delete [] m_aVec;


        n_Args = rOther.n_Args;
        m_aVec = new rtl_uString* [n_Args];
        for( i = 0; i < n_Args; ++i )
        {
            m_aVec[i] = rOther.m_aVec[i];
            rtl_uString_acquire( m_aVec[i] );
        }
    }

    return *this;
}

OArgumentList::~OArgumentList( )
{
    for( sal_uInt32 i = 0; i < n_Args; ++i ) rtl_uString_release( m_aVec[i] );
    delete[] m_aVec;
}


////////////////////////////////////////////////////////////////////////////////
/// Environment

OEnvironment::OEnvironment() :
    n_Vars( 0 ),
    m_aVec( 0 )
{
}

OEnvironment::OEnvironment( sal_uInt32 nVars, const NAMESPACE_RTL(OUString)* aArgument1, ... ) :
    n_Vars( nVars )
{
    m_aVec = new rtl_uString* [nVars];
    std::va_list pArgs;
    sal_uInt32 i = 0;
    const rtl::OUString* aArgument;

    va_start ( pArgs, aArgument1 );
    aArgument = aArgument1;

    while( true ) {
        m_aVec[i] = aArgument->pData;
        rtl_uString_acquire( m_aVec[i++] );
        if( i < n_Vars )
            aArgument = va_arg( pArgs,rtl::OUString* );
        else
            break;
    }
    va_end( pArgs );
}


OEnvironment::OEnvironment( const NAMESPACE_RTL(OUString) aVariableList[], sal_uInt32 nVars ) :
    n_Vars( nVars )
{
    m_aVec = new rtl_uString* [n_Vars];
    for( sal_uInt32 i = 0; i < n_Vars; ++ i )  {
        m_aVec[i] = aVariableList[i].pData;
        rtl_uString_acquire( m_aVec[i] );
    }
}

OEnvironment::OEnvironment( const OEnvironment& rOther ) : n_Vars( rOther.n_Vars )
{
    m_aVec = new rtl_uString* [n_Vars];

    sal_uInt32 i;
    for ( i = 0; i < n_Vars; ++i )
    {
        m_aVec[i] = rOther.m_aVec[i];
        rtl_uString_acquire( m_aVec[i] );
    }
}

OEnvironment& OEnvironment::operator=( const OEnvironment& rOther )
{
    if ( this != &rOther )
    {
        sal_uInt32 i;
        for ( i = 0; i < n_Vars; ++i )
            rtl_uString_release( m_aVec[i] );

        delete [] m_aVec;

        n_Vars = rOther.n_Vars;
        m_aVec = new rtl_uString* [n_Vars];
        for ( i = 0; i < n_Vars; ++i )
        {
            m_aVec[i] = rOther.m_aVec[i];
            rtl_uString_acquire( m_aVec[i] );
        }
    }

    return *this;
}

OEnvironment::~OEnvironment()
{
    for( sal_uInt32 i = 0; i < n_Vars; ++i ) rtl_uString_release( m_aVec[i] );
    delete[] m_aVec;
}

/////////////////////////////////////////////////////////////////////////////
// Process


VOS_IMPLEMENT_CLASSINFO(
    VOS_CLASSNAME(OProcess, vos),
    VOS_NAMESPACE(OProcess, vos),
    VOS_NAMESPACE(OObject, vos), 0);


OProcess::OProcess( ) :
    m_strImageName( ),
    m_strDirectory(),
    m_IoResources(0),
    m_NoResources(0),
    m_Process(0)
{
}


OProcess::OProcess( const rtl::OUString& strImageName ) :
    m_strImageName( strImageName ),
    m_strDirectory(),
    m_IoResources(0),
    m_NoResources(0),
    m_Process(0)
{
    // empty
}


OProcess::OProcess(const rtl::OUString& strImageName, const rtl::OUString& strWorkingDirectory) :
    m_strImageName( strImageName ),
    m_strDirectory( strWorkingDirectory ),
    m_IoResources(0),
    m_NoResources(0),
    m_Process(0)
{
    // empty
}


OProcess::~OProcess()
{
    osl_freeProcessHandle(m_Process);

    delete m_IoResources;
}

OProcess* OProcess::getProcess(TProcessIdentifier Identifier)
{
    oslProcess hProcess = osl_getProcess(Identifier);

    if (hProcess)
    {
        OProcess* pProcess = new OProcess( );

        pProcess->m_Process = hProcess;

        return pProcess;
    }

    return 0;
}


OProcess::TProcessError OProcess::execute(TProcessOption Options,
                                          const OArgumentList& aArgumentList,
                                          const OEnvironment&  aEnvironment )
{
    if (m_IoResources)
        m_IoResources[m_NoResources].Type = osl_Process_TypeNone;

    return ((TProcessError)osl_executeProcess(m_strImageName.pData,
                                              aArgumentList.m_aVec,
                                              aArgumentList.n_Args,
                                              Options,
                                              0,
                                              m_strDirectory.pData,
                                              aEnvironment.m_aVec,
                                              aEnvironment.n_Vars,
                                              m_IoResources,
                                              &m_Process));
}


OProcess::TProcessError OProcess::execute( TProcessOption Options,
                                           const OSecurity &Security,
                                           const OArgumentList& aArgumentList,
                                           const OEnvironment&  aEnvironment )
{
    if (m_IoResources)
        m_IoResources[m_NoResources].Type = osl_Process_TypeNone;

    return ((TProcessError)osl_executeProcess(m_strImageName.pData,
                                              aArgumentList.m_aVec,
                                              aArgumentList.n_Args,
                                              Options,
                                              Security,
                                              m_strDirectory.pData,
                                              aEnvironment.m_aVec,
                                              aEnvironment.n_Vars,
                                              m_IoResources,
                                              &m_Process));
}


OProcess::TProcessError OProcess::terminate()
{
   return (TProcessError)osl_terminateProcess(m_Process);
}

OProcess::TProcessError OProcess::getInfo(TProcessData Data, TProcessInfo* pInfo) const
{
   return (TProcessError)osl_getProcessInfo(m_Process, Data, pInfo);
}

OProcess::TProcessError OProcess::getCurrentInfo(TProcessData Data, TProcessInfo* pInfo)
{
   return (TProcessError)osl_getProcessInfo(0, Data, pInfo);
}

OProcess::TProcessError OProcess::join()
{
    return (TProcessError)osl_joinProcess(m_Process);
}


void OProcess::provideIOResource(oslSocket Socket, TDescriptorFlags Flags)
{
    if (! m_IoResources)
    {
        m_IoResources= new oslIOResource[MAX_RESOURCES + 1];
        m_NoResources = 0;
    }

    VOS_ASSERT(m_IoResources != 0);
    VOS_ASSERT(m_NoResources < MAX_RESOURCES);

    m_IoResources[m_NoResources].Type  = osl_Process_TypeSocket;
    m_IoResources[m_NoResources].Flags = (oslDescriptorFlag)Flags;
    m_IoResources[m_NoResources].Descriptor.Socket = Socket;

    m_NoResources++;
}


/*
OProcess::TProcessError OProcess::searchPath(const sal_Char* Name, sal_Char *Buffer, sal_uInt32 Max,
                                             const sal_Char* Path, sal_Char Separator)
{
    return (TProcessError)osl_searchPath(Name, Path, Separator, Buffer, Max);
}
*/

/////////////////////////////////////////////////////////////////////////////
// StartupInfo

VOS_IMPLEMENT_CLASSINFO(
    VOS_CLASSNAME(OStartupInfo, vos),
    VOS_NAMESPACE(OStartupInfo, vos),
    VOS_NAMESPACE(OObject, vos), 0);

OStartupInfo::OStartupInfo() :
    m_IoResources(0),
    m_NoResources(0)
{
}

OStartupInfo::~OStartupInfo()
{
    delete m_IoResources;
}

sal_Bool OStartupInfo::acceptIOResource(OSocket& rSocket)
{
    // jbu : functionality removed from vos
    return sal_False;
}

OStartupInfo::TStartupError OStartupInfo::getExecutableFile( rtl::OUString& strImageName )
{
    return (TStartupError) osl_getExecutableFile( &strImageName.pData );
}


OStartupInfo::TStartupError OStartupInfo::getCommandArg(sal_uInt32 nArg, rtl::OUString& strCommandArg)
{
    return ( TStartupError ) osl_getCommandArg( nArg,&strCommandArg.pData );
}

sal_uInt32 OStartupInfo::getCommandArgCount()
{
    return osl_getCommandArgCount();
}

OStartupInfo::TStartupError OStartupInfo::getEnvironment(const rtl::OUString& strVar,
                                                         rtl::OUString& strValue)
{
    return ( TStartupError ) osl_getEnvironment( strVar.pData, &strValue.pData );
}



/////////////////////////////////////////////////////////////////////////////
//
// OExtCommandLineImpl
//

namespace vos
{

class OExtCommandLineImpl
{
    void init();

    ::std::vector< ::rtl::OUString > aExtArgVector;
    sal_uInt32 m_nArgCount;

public:

    OExtCommandLineImpl();
    ~OExtCommandLineImpl();

    sal_uInt32 SAL_CALL getCommandArgCount();

    sal_Bool SAL_CALL getCommandArg(sal_uInt32 nArg, ::rtl::OUString& strCommandArg);
};

}

OExtCommandLineImpl::OExtCommandLineImpl()
    : m_nArgCount(0)
{
    init();
}

OExtCommandLineImpl::~OExtCommandLineImpl()
{

}


sal_uInt32 SAL_CALL OExtCommandLineImpl::getCommandArgCount()
{
    return m_nArgCount;
}


sal_Bool SAL_CALL OExtCommandLineImpl::getCommandArg(sal_uInt32 nArg, ::rtl::OUString& strCommandArg)
{
    if ( nArg >= m_nArgCount )
    {
        return sal_False;
    }

    strCommandArg = aExtArgVector[nArg];

    return sal_True;
}


void OExtCommandLineImpl::init()
{
    OStartupInfo aStartInfo;
    sal_uInt32 nIndex=0;
    sal_uInt32 nArgs = aStartInfo.getCommandArgCount();

    for ( nIndex = 0 ; nIndex < nArgs ; ++nIndex )
    {
        ::rtl::OUString aString;
        sal_Bool bRet = aStartInfo.getCommandArg(nIndex,aString);

        if ( aString[0] == (sal_Unicode) '@' )
        {
            ::rtl::OUString aFileName = aString.copy(1);
            ::osl::File aFile(aFileName);
            ::rtl::ByteSequence aSeq;

            ::osl::FileBase::RC aErr = aFile.open(OpenFlag_Read);

            if ( aErr != ::osl::FileBase::E_None )
            {
                break;
            }

            do
            {
                aErr = aFile.readLine(aSeq);
                if ( aSeq.getLength() != 0 )
                {
                    ::rtl::OUString newString((sal_Char*)aSeq.getArray(), aSeq.getLength(), RTL_TEXTENCODING_ASCII_US);
                    aExtArgVector.push_back( newString );
                    m_nArgCount++;
                }
            }
            while ( aErr == ::osl::FileBase::E_None && aSeq.getLength() > 0 );

            aFile.close();
            aFile.remove(aFileName);
        }
        else
        {
            aExtArgVector.push_back( aString );
            m_nArgCount++;
        }
    }
}



/////////////////////////////////////////////////////////////////////////////
//
// OExtCommandLine
//

OMutex OExtCommandLine::aMutex;
OExtCommandLineImpl* OExtCommandLine::pExtImpl=0;


VOS_IMPLEMENT_CLASSINFO(
    VOS_CLASSNAME(OExtCommandLine, vos),
    VOS_NAMESPACE(OExtCommandLine, vos),
    VOS_NAMESPACE(OObject, vos), 0);

OExtCommandLine::OExtCommandLine()
{
    OGuard Guard(aMutex);

    if ( pExtImpl == NULL )
    {
        pExtImpl = new OExtCommandLineImpl;
    }
}

OExtCommandLine::~OExtCommandLine()
{


}

sal_uInt32 SAL_CALL OExtCommandLine::getCommandArgCount()
{
    return pExtImpl->getCommandArgCount();
}


sal_Bool SAL_CALL OExtCommandLine::getCommandArg(sal_uInt32 nArg, ::rtl::OUString& strCommandArg)
{
    return pExtImpl->getCommandArg(nArg,strCommandArg);
}

