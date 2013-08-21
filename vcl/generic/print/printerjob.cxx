/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "psputil.hxx"
#include "glyphset.hxx"

#include "generic/printerjob.hxx"
#include "generic/printergfx.hxx"
#include "vcl/ppdparser.hxx"
#include "vcl/strhelper.hxx"
#include "vcl/printerinfomanager.hxx"

#include "rtl/ustring.hxx"
#include "rtl/strbuf.hxx"
#include "rtl/ustrbuf.hxx"

#include <osl/thread.h>
#include <osl/security.hxx>
#include <sal/alloca.h>
#include <sal/macros.h>

#include <algorithm>
#include <vector>

using namespace psp;


// forward declaration

#define nBLOCKSIZE 0x2000

namespace psp
{

sal_Bool
AppendPS (FILE* pDst, osl::File* pSrc, sal_uChar* pBuffer,
          sal_uInt32 nBlockSize = nBLOCKSIZE)
{
    if ((pDst == NULL) || (pSrc == NULL))
        return sal_False;

    if (pSrc->setPos(osl_Pos_Absolut, 0) != osl::FileBase::E_None)
        return sal_False;

    if (nBlockSize == 0)
        nBlockSize = nBLOCKSIZE;
    if (pBuffer == NULL)
        pBuffer = (sal_uChar*)alloca (nBlockSize);

    sal_uInt64 nIn = 0;
    sal_uInt64 nOut = 0;
    do
    {
        pSrc->read  (pBuffer, nBlockSize, nIn);
        if (nIn > 0)
            nOut = fwrite (pBuffer, 1, sal::static_int_cast<sal_uInt32>(nIn), pDst);
    }
    while ((nIn > 0) && (nIn == nOut));

    return sal_True;
}

} // namespace psp

/*
 * private convenience routines for file handling
 */

osl::File*
PrinterJob::CreateSpoolFile (const OUString& rName, const OUString& rExtension)
{
    osl::File*    pFile  = NULL;

    OUString aFile = rName + rExtension;
    OUString aFileURL;
    osl::File::RC nError = osl::File::getFileURLFromSystemPath( aFile, aFileURL );
    if (nError != osl::File::E_None)
        return NULL;
    aFileURL = maSpoolDirName + OUString("/") + aFileURL;

    pFile = new osl::File (aFileURL);
    nError = pFile->open (osl_File_OpenFlag_Read | osl_File_OpenFlag_Write | osl_File_OpenFlag_Create);
    if (nError != osl::File::E_None)
    {
        delete pFile;
        return NULL;
    }

    pFile->setAttributes (aFileURL,
                          osl_File_Attribute_OwnWrite | osl_File_Attribute_OwnRead);
    return pFile;
}

/*
 * public methods of PrinterJob: for use in PrinterGfx
 */

void
PrinterJob::GetScale (double &rXScale, double &rYScale) const
{
    rXScale = mfXScale;
    rYScale = mfYScale;
}

sal_uInt16
PrinterJob::GetDepth () const
{
    sal_Int32 nLevel = GetPostscriptLevel();
    sal_Bool  bColor = IsColorPrinter ();

    return nLevel > 1 && bColor ? 24 : 8;
}

sal_uInt16
PrinterJob::GetPostscriptLevel (const JobData *pJobData) const
{
    sal_uInt16 nPSLevel = 2;

    if( pJobData == NULL )
        pJobData = &m_aLastJobData;

    if( pJobData->m_nPSLevel )
        nPSLevel = pJobData->m_nPSLevel;
    else
        if( pJobData->m_pParser )
            nPSLevel = pJobData->m_pParser->getLanguageLevel();

    return nPSLevel;
}

sal_Bool
PrinterJob::IsColorPrinter () const
{
    sal_Bool bColor = sal_False;

    if( m_aLastJobData.m_nColorDevice )
        bColor = m_aLastJobData.m_nColorDevice == -1 ? sal_False : sal_True;
    else if( m_aLastJobData.m_pParser )
        bColor = m_aLastJobData.m_pParser->isColorDevice() ? sal_True : sal_False;

    return bColor;
}

osl::File*
PrinterJob::GetCurrentPageHeader ()
{
    return maHeaderList.back();
}

osl::File*
PrinterJob::GetCurrentPageBody ()
{
    return maPageList.back();
}

/*
 * public methods of PrinterJob: the actual job / spool handling
 */

PrinterJob::PrinterJob () :
        mpJobHeader( NULL ),
        mpJobTrailer( NULL ),
        m_bQuickJob( false )
{
}

/* remove all our temporary files, uses external program "rm", since
   osl functionality is inadequate */
void
removeSpoolDir (const OUString& rSpoolDir)
{
    OUString aSysPath;
    if( osl::File::E_None != osl::File::getSystemPathFromFileURL( rSpoolDir, aSysPath ) )
    {
        // Conversion did not work, as this is quite a dangerous action,
        // we should abort here ....
        OSL_FAIL( "psprint: couldn't remove spool directory" );
        return;
    }
    OString aSysPathByte =
        OUStringToOString (aSysPath, osl_getThreadTextEncoding());
    sal_Char  pSystem [128];
    sal_Int32 nChar = 0;

    nChar  = psp::appendStr ("rm -rf ",     pSystem);
    nChar += psp::appendStr (aSysPathByte.getStr(), pSystem + nChar);

    if (system (pSystem) == -1)
        OSL_FAIL( "psprint: couldn't remove spool directory" );
}

/* creates a spool directory with a "pidgin random" value based on
   current system time */
OUString
createSpoolDir ()
{
    TimeValue aCur;
    osl_getSystemTime( &aCur );
    sal_Int32 nRand = aCur.Seconds ^ (aCur.Nanosec/1000);

    OUString aTmpDir;
    osl_getTempDirURL( &aTmpDir.pData );

    do
    {
        OUStringBuffer aDir( aTmpDir.getLength() + 16 );
        aDir.append( aTmpDir );
        aDir.appendAscii( "/psp" );
        aDir.append(nRand);
        OUString aResult = aDir.makeStringAndClear();
        if( osl::Directory::create( aResult ) == osl::FileBase::E_None )
        {
            osl::File::setAttributes( aResult,
                                        osl_File_Attribute_OwnWrite
                                      | osl_File_Attribute_OwnRead
                                      | osl_File_Attribute_OwnExe );
            return aResult;
        }
        nRand++;
    } while( nRand );
    return OUString();
}

PrinterJob::~PrinterJob ()
{
    std::list< osl::File* >::iterator pPage;
    for (pPage = maPageList.begin(); pPage != maPageList.end(); ++pPage)
    {
        //(*pPage)->remove();
        delete *pPage;
    }
    for (pPage = maHeaderList.begin(); pPage != maHeaderList.end(); ++pPage)
    {
        //(*pPage)->remove();
        delete *pPage;
    }
    // mpJobHeader->remove();
    delete mpJobHeader;
    // mpJobTrailer->remove();
    delete mpJobTrailer;

    // XXX should really call osl::remove routines
    if( !maSpoolDirName.isEmpty() )
        removeSpoolDir (maSpoolDirName);

    // osl::Directory::remove (maSpoolDirName);
}

static void WriteLocalTimePS( osl::File *rFile )
{
    TimeValue m_start_time, tLocal;
    oslDateTime date_time;
    if (osl_getSystemTime( &m_start_time ) &&
        osl_getLocalTimeFromSystemTime( &m_start_time, &tLocal ) &&
        osl_getDateTimeFromTimeValue( &tLocal, &date_time ))
    {
        char ar[ 256 ];
        snprintf(
            ar, sizeof (ar),
            "%04d-%02d-%02d %02d:%02d:%02d ",
            date_time.Year, date_time.Month, date_time.Day,
            date_time.Hours, date_time.Minutes, date_time.Seconds );
        WritePS( rFile, ar );
    }
    else
        WritePS( rFile, "Unknown-Time" );
}

static bool isAscii( const OUString& rStr )
{
    sal_Int32 nLen = rStr.getLength();
    for( sal_Int32 i = 0; i < nLen; i++ )
        if( rStr[i] > 127 )
            return false;
    return true;
}

sal_Bool
PrinterJob::StartJob (
                      const OUString& rFileName,
                      int nMode,
                      const OUString& rJobName,
                      const OUString& rAppName,
                      const JobData& rSetupData,
                      PrinterGfx* pGraphics,
                      bool bIsQuickJob
                      )
{
    m_bQuickJob = bIsQuickJob;
    mnMaxWidthPt = mnMaxHeightPt = 0;
    mnLandscapes = mnPortraits = 0;
    m_pGraphics = pGraphics;
    InitPaperSize (rSetupData);

    // create file container for document header and trailer
    maFileName = rFileName;
    mnFileMode = nMode;
    maSpoolDirName = createSpoolDir ();
    maJobTitle = rJobName;

    OUString aExt(".ps");
    mpJobHeader  = CreateSpoolFile (OUString("psp_head"), aExt);
    mpJobTrailer = CreateSpoolFile (OUString("psp_tail"), aExt);
    if( ! (mpJobHeader && mpJobTrailer) ) // existing files are removed in destructor
        return sal_False;

    // write document header according to Document Structuring Conventions (DSC)
    WritePS (mpJobHeader,
             "%!PS-Adobe-3.0\n"
             "%%BoundingBox: (atend)\n" );

    OUString aFilterWS;

    // Creator (this application)
    aFilterWS = WhitespaceToSpace( rAppName, false );
    WritePS (mpJobHeader, "%%Creator: (");
    WritePS (mpJobHeader, aFilterWS);
    WritePS (mpJobHeader, ")\n");

    // For (user name)
    osl::Security aSecurity;
    OUString aUserName;
    if( aSecurity.getUserName( aUserName ) )
    {
        WritePS (mpJobHeader, "%%For: (");
        WritePS (mpJobHeader, aUserName);
        WritePS (mpJobHeader, ")\n");
    }

    // Creation Date (locale independent local time)
    WritePS (mpJobHeader, "%%CreationDate: (");
    WriteLocalTimePS (mpJobHeader);
    WritePS (mpJobHeader, ")\n");

    // Document Title
    /* #i74335#
    * The title should be clean ascii; rJobName however may
    * contain any Unicode character. So implement the following
    * algorithm:
    * use rJobName, if it contains only ascii
    * use the filename, if it contains only ascii
    * else omit %%Title
    */
    aFilterWS = WhitespaceToSpace( rJobName, false );
    OUString aTitle( aFilterWS );
    if( ! isAscii( aTitle ) )
    {
        sal_Int32 nIndex = 0;
        while( nIndex != -1 )
            aTitle = rFileName.getToken( 0, '/', nIndex );
        aTitle = WhitespaceToSpace( aTitle, false );
        if( ! isAscii( aTitle ) )
            aTitle = OUString();
    }

    maJobTitle = aFilterWS;
    if( !aTitle.isEmpty() )
    {
        WritePS (mpJobHeader, "%%Title: (");
        WritePS (mpJobHeader, aTitle);
        WritePS (mpJobHeader, ")\n");
    }

    // Language Level
    sal_Char pLevel[16];
    sal_Int32 nSz = getValueOf(GetPostscriptLevel(&rSetupData), pLevel);
    pLevel[nSz++] = '\n';
    pLevel[nSz  ] = '\0';
    WritePS (mpJobHeader, "%%LanguageLevel: ");
    WritePS (mpJobHeader, pLevel);

    // Other
    WritePS (mpJobHeader, "%%DocumentData: Clean7Bit\n");
    WritePS (mpJobHeader, "%%Pages: (atend)\n");
    WritePS (mpJobHeader, "%%Orientation: (atend)\n");
    WritePS (mpJobHeader, "%%PageOrder: Ascend\n");
    WritePS (mpJobHeader, "%%EndComments\n");

    // write Prolog
    writeProlog (mpJobHeader, rSetupData);

    // mark last job setup as not set
    m_aLastJobData.m_pParser = NULL;
    m_aLastJobData.m_aContext.setParser( NULL );

    return sal_True;
}

sal_Bool
PrinterJob::EndJob ()
{
    // no pages ? that really means no print job
    if( maPageList.empty() )
        return sal_False;

    // write document setup (done here because it
    // includes the accumulated fonts
    if( mpJobHeader )
        writeSetup( mpJobHeader, m_aDocumentJobData );
    m_pGraphics->OnEndJob();
    if( ! (mpJobHeader && mpJobTrailer) )
        return sal_False;

    // write document trailer according to Document Structuring Conventions (DSC)
    OStringBuffer aTrailer(512);
    aTrailer.append( "%%Trailer\n" );
    aTrailer.append( "%%BoundingBox: 0 0 " );
    aTrailer.append( (sal_Int32)mnMaxWidthPt );
    aTrailer.append( " " );
    aTrailer.append( (sal_Int32)mnMaxHeightPt );
    if( mnLandscapes > mnPortraits )
        aTrailer.append("\n%%Orientation: Landscape");
    else
        aTrailer.append("\n%%Orientation: Portrait");
    aTrailer.append( "\n%%Pages: " );
    aTrailer.append( (sal_Int32)maPageList.size() );
    aTrailer.append( "\n%%EOF\n" );
    WritePS (mpJobTrailer, aTrailer.getStr());

    /*
     * spool the set of files to their final destination, this is U**X dependent
     */

    FILE* pDestFILE = NULL;

    /* create a destination either as file or as a pipe */
    sal_Bool bSpoolToFile = !maFileName.isEmpty();
    if (bSpoolToFile)
    {
        const OString aFileName = OUStringToOString (maFileName,
                                                               osl_getThreadTextEncoding());
        if( mnFileMode )
        {
            int nFile = open( aFileName.getStr(), O_CREAT | O_EXCL | O_RDWR, mnFileMode );
            if( nFile != -1 )
            {
                pDestFILE = fdopen( nFile, "w" );
                if( pDestFILE == NULL )
                {
                    close( nFile );
                    unlink( aFileName.getStr() );
                    return sal_False;
                }
            }
            else
                chmod( aFileName.getStr(), mnFileMode );
        }
        if (pDestFILE == NULL)
            pDestFILE = fopen (aFileName.getStr(), "w");

        if (pDestFILE == NULL)
            return sal_False;
    }
    else
    {
        PrinterInfoManager& rPrinterInfoManager = PrinterInfoManager::get ();
        pDestFILE = rPrinterInfoManager.startSpool( m_aLastJobData.m_aPrinterName, m_bQuickJob );
        if (pDestFILE == NULL)
            return sal_False;
    }

    /* spool the document parts to the destination */

    sal_uChar pBuffer[ nBLOCKSIZE ];

    AppendPS (pDestFILE, mpJobHeader, pBuffer);
    mpJobHeader->close();

    sal_Bool bSuccess = sal_True;
    std::list< osl::File* >::iterator pPageBody;
    std::list< osl::File* >::iterator pPageHead;
    for (pPageBody  = maPageList.begin(), pPageHead  = maHeaderList.begin();
         pPageBody != maPageList.end() && pPageHead != maHeaderList.end();
         ++pPageBody, ++pPageHead)
    {
        if( *pPageHead )
        {
            osl::File::RC nError = (*pPageHead)->open(osl_File_OpenFlag_Read);
            if (nError == osl::File::E_None)
            {
                AppendPS (pDestFILE, *pPageHead, pBuffer);
                (*pPageHead)->close();
            }
        }
        else
            bSuccess = sal_False;
        if( *pPageBody )
        {
            osl::File::RC nError = (*pPageBody)->open(osl_File_OpenFlag_Read);
            if (nError == osl::File::E_None)
            {
                AppendPS (pDestFILE, *pPageBody, pBuffer);
                (*pPageBody)->close();
            }
        }
        else
            bSuccess = sal_False;
    }

    AppendPS (pDestFILE, mpJobTrailer, pBuffer);
    mpJobTrailer->close();

    /* well done */

    if (bSpoolToFile)
        fclose (pDestFILE);
    else
    {
        PrinterInfoManager& rPrinterInfoManager = PrinterInfoManager::get();
        if (0 == rPrinterInfoManager.endSpool( m_aLastJobData.m_aPrinterName,
            maJobTitle, pDestFILE, m_aDocumentJobData, true ))
        {
            bSuccess = sal_False;
        }
    }

    return bSuccess;
}

sal_Bool
PrinterJob::AbortJob ()
{
    m_pGraphics->OnEndJob();
    return sal_False;
}

void
PrinterJob::InitPaperSize (const JobData& rJobSetup)
{
    int nRes = rJobSetup.m_aContext.getRenderResolution ();

    OUString aPaper;
    int nWidth, nHeight;
    rJobSetup.m_aContext.getPageSize (aPaper, nWidth, nHeight);

    int nLeft = 0, nRight = 0, nUpper = 0, nLower = 0;
    const PPDParser* pParser = rJobSetup.m_aContext.getParser();
    if (pParser != NULL)
        pParser->getMargins (aPaper, nLeft, nRight, nUpper, nLower);

    mnResolution    = nRes;

    mnWidthPt       = nWidth;
    mnHeightPt      = nHeight;

    if( mnWidthPt > mnMaxWidthPt )
        mnMaxWidthPt = mnWidthPt;
    if( mnHeightPt > mnMaxHeightPt )
        mnMaxHeightPt = mnHeightPt;

    mnLMarginPt     = nLeft;
    mnRMarginPt     = nRight;
    mnTMarginPt     = nUpper;
    mnBMarginPt     = nLower;

    mfXScale        = (double)72.0 / (double)mnResolution;
    mfYScale        = -1.0 * (double)72.0 / (double)mnResolution;
}


sal_Bool
PrinterJob::StartPage (const JobData& rJobSetup)
{
    InitPaperSize (rJobSetup);

    OUString aPageNo = OUString::number ((sal_Int32)maPageList.size()+1); // sequential page number must start with 1
    OUString aExt    = aPageNo + OUString(".ps");

    osl::File* pPageHeader = CreateSpoolFile ( OUString("psp_pghead"), aExt);
    osl::File* pPageBody   = CreateSpoolFile ( OUString("psp_pgbody"), aExt);

    maHeaderList.push_back (pPageHeader);
    maPageList.push_back (pPageBody);

    if( ! (pPageHeader && pPageBody) )
        return sal_False;

    // write page header according to Document Structuring Conventions (DSC)
    WritePS (pPageHeader, "%%Page: ");
    WritePS (pPageHeader, aPageNo);
    WritePS (pPageHeader, " ");
    WritePS (pPageHeader, aPageNo);
    WritePS (pPageHeader, "\n");

    if( rJobSetup.m_eOrientation == orientation::Landscape )
    {
        WritePS (pPageHeader, "%%PageOrientation: Landscape\n");
        mnLandscapes++;
    }
    else
    {
        WritePS (pPageHeader, "%%PageOrientation: Portrait\n");
        mnPortraits++;
    }

    sal_Char  pBBox [256];
    sal_Int32 nChar = 0;

    nChar  = psp::appendStr  ("%%PageBoundingBox: ",    pBBox);
    nChar += psp::getValueOf (mnLMarginPt,              pBBox + nChar);
    nChar += psp::appendStr  (" ",                      pBBox + nChar);
    nChar += psp::getValueOf (mnBMarginPt,              pBBox + nChar);
    nChar += psp::appendStr  (" ",                      pBBox + nChar);
    nChar += psp::getValueOf (mnWidthPt  - mnRMarginPt, pBBox + nChar);
    nChar += psp::appendStr  (" ",                      pBBox + nChar);
    nChar += psp::getValueOf (mnHeightPt - mnTMarginPt, pBBox + nChar);
    nChar += psp::appendStr  ("\n",                     pBBox + nChar);

    WritePS (pPageHeader, pBBox);

    /* #i7262# #i65491# write setup only before first page
     *  (to %%Begin(End)Setup, instead of %%Begin(End)PageSetup)
     *  don't do this in StartJob since the jobsetup there may be
     *  different.
     */
    bool bWriteFeatures = true;
    if( 1 == maPageList.size() )
    {
        m_aDocumentJobData = rJobSetup;
        bWriteFeatures = false;
    }

    if ( writePageSetup( pPageHeader, rJobSetup, bWriteFeatures ) )
    {
        m_aLastJobData = rJobSetup;
        return true;
    }

    return false;
}

sal_Bool
PrinterJob::EndPage ()
{
    m_pGraphics->OnEndPage();

    osl::File* pPageHeader = maHeaderList.back();
    osl::File* pPageBody   = maPageList.back();

    if( ! (pPageBody && pPageHeader) )
        return sal_False;

    // copy page to paper and write page trailer according to DSC

    sal_Char pTrailer[256];
    sal_Int32 nChar = 0;
    nChar  = psp::appendStr ("grestore grestore\n", pTrailer);
    nChar += psp::appendStr ("showpage\n",          pTrailer + nChar);
    nChar += psp::appendStr ("%%PageTrailer\n\n",   pTrailer + nChar);
    WritePS (pPageBody, pTrailer);

    // this page is done for now, close it to avoid having too many open fd's

    pPageHeader->close();
    pPageBody->close();

    return sal_True;
}

struct less_ppd_key : public ::std::binary_function<double, double, bool>
{
    bool operator()(const PPDKey* left, const PPDKey* right)
    { return left->getOrderDependency() < right->getOrderDependency(); }
};

static bool writeFeature( osl::File* pFile, const PPDKey* pKey, const PPDValue* pValue, bool bUseIncluseFeature )
{
    if( ! pKey || ! pValue )
        return true;

    OStringBuffer aFeature(256);
    aFeature.append( "[{\n" );
    if( bUseIncluseFeature )
        aFeature.append( "%%IncludeFeature:" );
    else
        aFeature.append( "%%BeginFeature:" );
    aFeature.append( " *" );
    aFeature.append( OUStringToOString( pKey->getKey(), RTL_TEXTENCODING_ASCII_US ) );
    aFeature.append( ' ' );
    aFeature.append( OUStringToOString( pValue->m_aOption, RTL_TEXTENCODING_ASCII_US ) );
    if( !bUseIncluseFeature )
    {
        aFeature.append( '\n' );
        aFeature.append( OUStringToOString( pValue->m_aValue, RTL_TEXTENCODING_ASCII_US ) );
        aFeature.append( "\n%%EndFeature" );
    }
    aFeature.append( "\n} stopped cleartomark\n" );
    sal_uInt64 nWritten = 0;
    return pFile->write( aFeature.getStr(), aFeature.getLength(), nWritten )
        || nWritten != (sal_uInt64)aFeature.getLength() ? false : true;
}

bool PrinterJob::writeFeatureList( osl::File* pFile, const JobData& rJob, bool bDocumentSetup )
{
    bool bSuccess = true;

    // emit features ordered to OrderDependency
    // ignore features that are set to default

    // sanity check
    if( rJob.m_pParser == rJob.m_aContext.getParser() &&
        rJob.m_pParser &&
        ( m_aLastJobData.m_pParser == rJob.m_pParser || m_aLastJobData.m_pParser == NULL )
        )
    {
        int i;
        int nKeys = rJob.m_aContext.countValuesModified();
        ::std::vector< const PPDKey* > aKeys( nKeys );
        for(  i = 0; i < nKeys; i++ )
            aKeys[i] = rJob.m_aContext.getModifiedKey( i );
        ::std::sort( aKeys.begin(), aKeys.end(), less_ppd_key() );

        for( i = 0; i < nKeys && bSuccess; i++ )
        {
            const PPDKey* pKey = aKeys[i];
            bool bEmit = false;
            if( bDocumentSetup )
            {
                if( pKey->getSetupType()    == PPDKey::DocumentSetup )
                    bEmit = true;
            }
            if( pKey->getSetupType()    == PPDKey::PageSetup        ||
                pKey->getSetupType()    == PPDKey::AnySetup )
                bEmit = true;
            if( bEmit )
            {
                const PPDValue* pValue = rJob.m_aContext.getValue( pKey );
                if( pValue
                    && pValue->m_eType == eInvocation
                    && ( m_aLastJobData.m_pParser == NULL
                         || m_aLastJobData.m_aContext.getValue( pKey ) != pValue
                         || bDocumentSetup
                         )
                   )
                {
                    // try to avoid PS level 2 feature commands if level is set to 1
                    if( GetPostscriptLevel( &rJob ) == 1 )
                    {
                        bool bHavePS2 =
                            ( pValue->m_aValue.indexOf( "<<" ) != -1 )
                            ||
                            ( pValue->m_aValue.indexOf( ">>" ) != -1 );
                        if( bHavePS2 )
                            continue;
                    }
                    bSuccess = writeFeature( pFile, pKey, pValue, PrinterInfoManager::get().getUseIncludeFeature() );
                }
            }
        }
    }
    else
        bSuccess = false;

    return bSuccess;
}

bool PrinterJob::writePageSetup( osl::File* pFile, const JobData& rJob, bool bWriteFeatures )
{
    bool bSuccess = true;

    WritePS (pFile, "%%BeginPageSetup\n%\n");
    if ( bWriteFeatures )
        bSuccess = writeFeatureList( pFile, rJob, false );
    WritePS (pFile, "%%EndPageSetup\n");

    sal_Char  pTranslate [128];
    sal_Int32 nChar = 0;

    if( rJob.m_eOrientation == orientation::Portrait )
    {
        nChar  = psp::appendStr  ("gsave\n[",   pTranslate);
        nChar += psp::getValueOfDouble (        pTranslate + nChar, mfXScale, 5);
        nChar += psp::appendStr  (" 0 0 ",      pTranslate + nChar);
        nChar += psp::getValueOfDouble (        pTranslate + nChar, mfYScale, 5);
        nChar += psp::appendStr  (" ",          pTranslate + nChar);
        nChar += psp::getValueOf (mnRMarginPt,  pTranslate + nChar);
        nChar += psp::appendStr  (" ",          pTranslate + nChar);
        nChar += psp::getValueOf (mnHeightPt-mnTMarginPt,
                                  pTranslate + nChar);
        nChar += psp::appendStr  ("] concat\ngsave\n",
                                  pTranslate + nChar);
    }
    else
    {
        nChar  = psp::appendStr  ("gsave\n",    pTranslate);
        nChar += psp::appendStr  ("[ 0 ",       pTranslate + nChar);
        nChar += psp::getValueOfDouble (        pTranslate + nChar, -mfYScale, 5);
        nChar += psp::appendStr  (" ",          pTranslate + nChar);
        nChar += psp::getValueOfDouble (        pTranslate + nChar, mfXScale, 5);
        nChar += psp::appendStr  (" 0 ",        pTranslate + nChar );
        nChar += psp::getValueOfDouble (        pTranslate + nChar, mnLMarginPt, 5 );
        nChar += psp::appendStr  (" ",          pTranslate + nChar);
        nChar += psp::getValueOf (mnBMarginPt,  pTranslate + nChar );
        nChar += psp::appendStr ("] concat\ngsave\n",
                                 pTranslate + nChar);
    }

    WritePS (pFile, pTranslate);

    return bSuccess;
}

void PrinterJob::writeJobPatch( osl::File* pFile, const JobData& rJobData )
{
    if( ! PrinterInfoManager::get().getUseJobPatch() )
        return;

    const PPDKey* pKey = NULL;

    if( rJobData.m_pParser )
        pKey = rJobData.m_pParser->getKey( OUString( "JobPatchFile"  ) );
    if( ! pKey )
        return;

    // order the patch files
    // according to PPD spec the JobPatchFile options must be int
    // and should be emitted in order
    std::list< sal_Int32 > patch_order;
    int nValueCount = pKey->countValues();
    for( int i = 0; i < nValueCount; i++ )
    {
        const PPDValue* pVal = pKey->getValue( i );
        patch_order.push_back( pVal->m_aOption.toInt32() );
        if( patch_order.back() == 0 && ! pVal->m_aOption.equalsAscii( "0" ) )
        {
            WritePS( pFile, "% Warning: left out JobPatchFile option \"" );
            OString aOption = OUStringToOString( pVal->m_aOption, RTL_TEXTENCODING_ASCII_US );
            WritePS( pFile, aOption.getStr() );
            WritePS( pFile,
                     "\"\n% as it violates the PPD spec;\n"
                     "% JobPatchFile options need to be numbered for ordering.\n" );
        }
    }

    patch_order.sort();
    patch_order.unique();

    while( patch_order.begin() != patch_order.end() )
    {
        // note: this discards patch files not adhering to the "int" scheme
        // as there won't be a value for them
        writeFeature( pFile, pKey, pKey->getValue( OUString::number( patch_order.front() ) ), false );
        patch_order.pop_front();
    }
}

bool PrinterJob::writeProlog (osl::File* pFile, const JobData& rJobData )
{
    WritePS( pFile, "%%BeginProlog\n" );

    // JobPatchFile feature needs to be emitted at begin of prolog
    writeJobPatch( pFile, rJobData );

    static const sal_Char pProlog[] = {
        "%%BeginResource: procset PSPrint-Prolog 1.0 0\n"
        "/ISO1252Encoding [\n"
        "/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef\n"
        "/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef\n"
        "/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef\n"
        "/.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef\n"
        "/space /exclam /quotedbl /numbersign /dollar /percent /ampersand /quotesingle\n"
        "/parenleft /parenright /asterisk /plus /comma /hyphen /period /slash\n"
        "/zero /one /two /three /four /five /six /seven\n"
        "/eight /nine /colon /semicolon /less /equal /greater /question\n"
        "/at /A /B /C /D /E /F /G\n"
        "/H /I /J /K /L /M /N /O\n"
        "/P /Q /R /S /T /U /V /W\n"
        "/X /Y /Z /bracketleft /backslash /bracketright /asciicircum /underscore\n"
        "/grave /a /b /c /d /e /f /g\n"
        "/h /i /j /k /l /m /n /o\n"
        "/p /q /r /s /t /u /v /w\n"
        "/x /y /z /braceleft /bar /braceright /asciitilde /unused\n"
        "/Euro /unused /quotesinglbase /florin /quotedblbase /ellipsis /dagger /daggerdbl\n"
        "/circumflex /perthousand /Scaron /guilsinglleft /OE /unused /Zcaron /unused\n"
        "/unused /quoteleft /quoteright /quotedblleft /quotedblright /bullet /endash /emdash\n"
        "/tilde /trademark /scaron /guilsinglright /oe /unused /zcaron /Ydieresis\n"
        "/space /exclamdown /cent /sterling /currency /yen /brokenbar /section\n"
        "/dieresis /copyright /ordfeminine /guillemotleft /logicalnot /hyphen /registered /macron\n"
        "/degree /plusminus /twosuperior /threesuperior /acute /mu /paragraph /periodcentered\n"
        "/cedilla /onesuperior /ordmasculine /guillemotright /onequarter /onehalf /threequarters /questiondown\n"
        "/Agrave /Aacute /Acircumflex /Atilde /Adieresis /Aring /AE /Ccedilla\n"
        "/Egrave /Eacute /Ecircumflex /Edieresis /Igrave /Iacute /Icircumflex /Idieresis\n"
        "/Eth /Ntilde /Ograve /Oacute /Ocircumflex /Otilde /Odieresis /multiply\n"
        "/Oslash /Ugrave /Uacute /Ucircumflex /Udieresis /Yacute /Thorn /germandbls\n"
        "/agrave /aacute /acircumflex /atilde /adieresis /aring /ae /ccedilla\n"
        "/egrave /eacute /ecircumflex /edieresis /igrave /iacute /icircumflex /idieresis\n"
        "/eth /ntilde /ograve /oacute /ocircumflex /otilde /odieresis /divide\n"
        "/oslash /ugrave /uacute /ucircumflex /udieresis /yacute /thorn /ydieresis] def\n"
        "\n"
        "/psp_definefont { exch dup findfont dup length dict begin { 1 index /FID ne\n"
        "{ def } { pop pop } ifelse } forall /Encoding 3 -1 roll def\n"
        "currentdict end exch pop definefont pop } def\n"
        "\n"
        "/pathdict dup 8 dict def load begin\n"
        "/rcmd { { currentfile 1 string readstring pop 0 get dup 32 gt { exit }\n"
        "{ pop } ifelse } loop dup 126 eq { pop exit } if 65 sub dup 16#3 and 1\n"
        "add exch dup 16#C and -2 bitshift 16#3 and 1 add exch 16#10 and 16#10\n"
        "eq 3 1 roll exch } def\n"
        "/rhex { dup 1 sub exch currentfile exch string readhexstring pop dup 0\n"
        "get dup 16#80 and 16#80 eq dup 3 1 roll { 16#7f and } if 2 index 0 3\n"
        "-1 roll put 3 1 roll 0 0 1 5 -1 roll { 2 index exch get add 256 mul }\n"
        "for 256 div exch pop exch { neg } if } def\n"
        "/xcmd { rcmd exch rhex exch rhex exch 5 -1 roll add exch 4 -1 roll add\n"
        "1 index 1 index 5 -1 roll { moveto } { lineto } ifelse } def end\n"
        "/readpath { 0 0 pathdict begin { xcmd } loop end pop pop } def\n"
        "\n"
        "systemdict /languagelevel known not {\n"
        "/xshow { exch dup length 0 1 3 -1 roll 1 sub { dup 3 index exch get\n"
        "exch 2 index exch get 1 string dup 0 4 -1 roll put currentpoint 3 -1\n"
        "roll show moveto 0 rmoveto } for pop pop } def\n"
        "/rectangle { 4 -2 roll moveto 1 index 0 rlineto 0 exch rlineto neg 0\n"
        "rlineto closepath } def\n"
        "/rectfill { rectangle fill } def\n"
        "/rectstroke { rectangle stroke } def } if\n"
        "/bshow { currentlinewidth 3 1 roll currentpoint 3 index show moveto\n"
        "setlinewidth false charpath stroke setlinewidth } def\n"
        "/bxshow { currentlinewidth 4 1 roll setlinewidth exch dup length 1 sub\n"
        "0 1 3 -1 roll { 1 string 2 index 2 index get 1 index exch 0 exch put dup\n"
        "currentpoint 3 -1 roll show moveto currentpoint 3 -1 roll false charpath\n"
        "stroke moveto 2 index exch get 0 rmoveto } for pop pop setlinewidth } def\n"
        "\n"
        "/psp_lzwfilter { currentfile /ASCII85Decode filter /LZWDecode filter } def\n"
        "/psp_ascii85filter { currentfile /ASCII85Decode filter } def\n"
        "/psp_lzwstring { psp_lzwfilter 1024 string readstring } def\n"
        "/psp_ascii85string { psp_ascii85filter 1024 string readstring } def\n"
        "/psp_imagedict {\n"
        "/psp_bitspercomponent { 3 eq { 1 }{ 8 } ifelse } def\n"
        "/psp_decodearray { [ [0 1 0 1 0 1] [0 255] [0 1] [0 255] ] exch get }\n"
        "def 7 dict dup\n"
        "/ImageType 1 put dup\n"
        "/Width 7 -1 roll put dup\n"
        "/Height 5 index put dup\n"
        "/BitsPerComponent 4 index psp_bitspercomponent put dup\n"
        "/Decode 5 -1 roll psp_decodearray put dup\n"
        "/ImageMatrix [1 0 0 1 0 0] dup 5 8 -1 roll put put dup\n"
        "/DataSource 4 -1 roll 1 eq { psp_lzwfilter } { psp_ascii85filter } ifelse put\n"
        "} def\n"
        "%%EndResource\n"
        "%%EndProlog\n"
    };
    WritePS (pFile, pProlog);

    return true;
}

bool PrinterJob::writeSetup( osl::File* pFile, const JobData& rJob )
{
    WritePS (pFile, "%%BeginSetup\n%\n");

    // download fonts
    std::list< OString > aFonts[2];
    m_pGraphics->writeResources( pFile, aFonts[0], aFonts[1] );

    for( int i = 0; i < 2; i++ )
    {
        if( !aFonts[i].empty() )
        {
            std::list< OString >::const_iterator it = aFonts[i].begin();
            OStringBuffer aLine( 256 );
            if( i == 0 )
                aLine.append( "%%DocumentSuppliedResources: font " );
            else
                aLine.append( "%%DocumentNeededResources: font " );
            aLine.append( *it );
            aLine.append( "\n" );
            WritePS ( pFile, aLine.getStr() );
            while( (++it) != aFonts[i].end() )
            {
                aLine.setLength(0);
                aLine.append( "%%+ font " );
                aLine.append( *it );
                aLine.append( "\n" );
                WritePS ( pFile, aLine.getStr() );
            }
        }
    }

    bool bSuccess = true;
    // in case of external print dialog the number of copies is prepended
    // to the job, let us not complicate things by emitting our own copy count
    bool bExternalDialog = PrinterInfoManager::get().checkFeatureToken( GetPrinterName(), "external_dialog" );
    if( ! bExternalDialog && rJob.m_nCopies > 1 )
    {
        // setup code
        OStringBuffer aLine("/#copies ");
        aLine.append(static_cast<sal_Int32>(rJob.m_nCopies));
        aLine.append(" def\n");
        sal_uInt64 nWritten = 0;
        bSuccess = pFile->write(aLine.getStr(), aLine.getLength(), nWritten)
            || nWritten != static_cast<sal_uInt64>(aLine.getLength()) ?
             false : true;

        if( bSuccess && GetPostscriptLevel( &rJob ) >= 2 )
            WritePS (pFile, "<< /NumCopies null /Policies << /NumCopies 1 >> >> setpagedevice\n" );
    }

    bool bFeatureSuccess = writeFeatureList( pFile, rJob, true );

    WritePS (pFile, "%%EndSetup\n");

    return bSuccess && bFeatureSuccess;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
