/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2009, 2010.
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

#include "vbafilesearch.hxx"
#include "vbaapplication.hxx"
#include "vbafoundfiles.hxx"
#include <comphelper/processfactory.hxx>
#include <tools/urlobj.hxx>
#include <tools/wldcrd.hxx>
#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <vector>
#include "unotools/viewoptions.hxx"
#include <osl/file.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::lang;
using namespace comphelper;

static Reference< XSimpleFileAccess3 > getFileAccess( void )
{
    static Reference< XSimpleFileAccess3 > xSFI;
    if( !xSFI.is() )
    {
        Reference< XMultiServiceFactory > xSMgr = getProcessServiceFactory();
        if( xSMgr.is() )
        {
            xSFI = Reference< XSimpleFileAccess3 >( xSMgr->createInstance
                ( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.SimpleFileAccess" )) ), UNO_QUERY );
        }
    }
    return xSFI;
}

ScVbaFileSearch::ScVbaFileSearch( ScVbaApplication* pApp, const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext )
        : ScVbaFileSearchImpl_BASE( xParent, xContext ), m_pApplication( pApp )
{
    NewSearch();
}

ScVbaFileSearch::~ScVbaFileSearch()
{
}

::rtl::OUString SAL_CALL ScVbaFileSearch::getFileName() throw (css::uno::RuntimeException)
{
    return m_sFileName;
}

void SAL_CALL ScVbaFileSearch::setFileName(const ::rtl::OUString& _fileName ) throw (css::uno::RuntimeException)
{
    m_sFileName = _fileName;
}

::rtl::OUString SAL_CALL ScVbaFileSearch::getLookIn() throw (css::uno::RuntimeException)
{
    return m_sLookIn;
}

void SAL_CALL ScVbaFileSearch::setLookIn( const ::rtl::OUString& _lookIn ) throw (css::uno::RuntimeException)
{
    m_sLookIn = _lookIn;
}

sal_Bool SAL_CALL ScVbaFileSearch::getSearchSubFolders() throw (css::uno::RuntimeException)
{
    return m_bSearchSubFolders;
}

void SAL_CALL ScVbaFileSearch::setSearchSubFolders( sal_Bool _searchSubFolders ) throw (css::uno::RuntimeException)
{
    m_bSearchSubFolders = _searchSubFolders;
}

sal_Bool SAL_CALL ScVbaFileSearch::getMatchTextExactly() throw (css::uno::RuntimeException)
{
    return m_bMatchTextExactly;
}

void SAL_CALL ScVbaFileSearch::setMatchTextExactly( sal_Bool _matchTextExactly ) throw (css::uno::RuntimeException)
{
    m_bMatchTextExactly = _matchTextExactly;
}

static bool IsWildCard( const ::rtl::OUString& fileName )
{
    static sal_Char cWild1 = '*';
    static sal_Char cWild2 = '?';

    return  ( ( fileName.indexOf( cWild1 ) >= 0 )
            || ( fileName.indexOf( cWild2 ) >= 0 ) );
}

static void SearchWildCard(const WildCard& wildCard, const ::rtl::OUString& aDir, bool bSearchSubFolders, css::uno::Sequence< rtl::OUString >& aSearchedFiles)
{
    Reference< XSimpleFileAccess3 > xSFI = getFileAccess();
    Sequence< rtl::OUString > aDirSeq;
    try
    {
        if ( xSFI.is() )
        {
            aDirSeq = xSFI->getFolderContents( aDir, bSearchSubFolders );
        }
    }
    catch( css::uno::Exception& )
    {
    }
    sal_Int32 nLength = aDirSeq.getLength();
    for ( sal_Int32 i = 0; i < nLength; i++ )
    {
        rtl::OUString aURLStr = aDirSeq[i];
        if ( xSFI->isFolder( aURLStr ) )
        {
            if ( bSearchSubFolders )
            {
                SearchWildCard( wildCard, aURLStr, true, aSearchedFiles );
            }
        }
        else
        {
            INetURLObject aFileURL( aURLStr );
            rtl::OUString aFileName = aFileURL.GetLastName( INetURLObject::DECODE_UNAMBIGUOUS );
            if ( wildCard.Matches( aFileName.toAsciiLowerCase() ) )
            {
                sal_Int32 nFilesLength = aSearchedFiles.getLength();
                aSearchedFiles.realloc( nFilesLength + 1 );
                rtl::OUString sSystemPath;
                ::osl::File::getSystemPathFromFileURL( aURLStr, sSystemPath );
                aSearchedFiles[nFilesLength] = sSystemPath;
            }
        }
    }
}

sal_Int32 SAL_CALL ScVbaFileSearch::Execute( )  throw (css::uno::RuntimeException)
{
    m_aSearchedFiles.realloc(0);
    Reference< XSimpleFileAccess3 > xSFI = getFileAccess();
    if ( !xSFI.is() || !xSFI->isFolder( m_sLookIn ) )
    {
        return 0;
    }

    if ( m_sFileName == ::rtl::OUString() )
    {
        return 1;
    }

    ::rtl::OUString aTempFileName = m_sFileName.toAsciiLowerCase();
    if ( IsWildCard( aTempFileName ) )
    {
        bool bEndWithAsterisk = aTempFileName.endsWithAsciiL("*", 1);
        bool bStartWithAsterisk = (aTempFileName.indexOf(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("*"))) == 0);
        if ( !bEndWithAsterisk && !bStartWithAsterisk )
        {
            aTempFileName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("*")) + aTempFileName + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("*"));
        }
    }
    else
    {
        aTempFileName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("*")) + aTempFileName + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("*"));
    }
    WildCard wildCard( aTempFileName );
    SearchWildCard( wildCard, m_sLookIn, m_bSearchSubFolders, m_aSearchedFiles );

    return m_aSearchedFiles.getLength();
}

// set ScVbaApplication::getDefaultFilePath(  ) as the InitPath for FileSearch
 ::rtl::OUString ScVbaFileSearch::getInitPath() throw (css::uno::RuntimeException)
{
    String aPath;

    if (m_pApplication != NULL)
    {
        aPath = m_pApplication->getDefaultFilePath();
    }

    return aPath;
}

void SAL_CALL ScVbaFileSearch::NewSearch( )  throw (css::uno::RuntimeException)
{
    m_sFileName = ::rtl::OUString();
    m_sLookIn = getInitPath();
    m_bSearchSubFolders = false;
    m_bMatchTextExactly = false;
    m_aSearchedFiles.realloc(0);
}

Reference< XFoundFiles > SAL_CALL ScVbaFileSearch::getFoundFiles() throw (css::uno::RuntimeException)
{
    css::uno::Reference< ov::XFoundFiles > xFoundFiles = new VbaFoundFiles(
        mxParent, mxContext, (css::container::XIndexAccess *) new VbaFoundFilesEnum(m_aSearchedFiles) );
    return xFoundFiles;
}

rtl::OUString& ScVbaFileSearch::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("VbaFileSearch") );
    return sImplName;
}

css::uno::Sequence< rtl::OUString > ScVbaFileSearch::getServiceNames()
{
    static css::uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.FileSearch") );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
