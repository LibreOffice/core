/*************************************************************************
 *
 *  $RCSfile: smdll.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:53:21 $
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

#pragma hdrstop

#ifndef _SOT_FACTORY_HXX //autogen
#include <sot/factory.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#ifndef _SVX_MODCTRL_HXX //autogen
#include <svx/modctrl.hxx>
#endif
#ifndef _SVX_ZOOMCTRL_HXX //autogen
#include <svx/zoomctrl.hxx>
#endif
#ifndef _SFX_OBJFAC_HXX
#include <sfx2/docfac.hxx>
#endif
#ifndef _SVX_LBOXCTRL_HXX_
#include <svx/lboxctrl.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

#ifndef _SMDLL_HXX
#include <smdll.hxx>
#endif
#ifndef DOCUMENT_HXX
#include <document.hxx>
#endif
#ifndef TOOLBOX_HXX
#include <toolbox.hxx>
#endif
#ifndef VIEW_HXX
#include <view.hxx>
#endif

#ifndef _STARMATH_HRC
#include <starmath.hrc>
#endif


BOOL SmDLL::bInitialized = FALSE;

/*************************************************************************
|*
|* Initialisierung
|*
\************************************************************************/
void SmDLL::Init()
{
    if ( bInitialized )
        return;

    bInitialized = TRUE;

    // called directly after loading the DLL
    // do whatever you want, you may use Sd-DLL too

    // the SdModule must be created

    SmModule** ppShlPtr = (SmModule**) GetAppData(SHL_SM);
    *ppShlPtr = new SmModule( &SmDocShell::Factory() );

    SmDocShell::RegisterFactory(SDT_SMA_DOCFACTPRIO);
    //SmDocShell::InitFactory();

    SfxModule *p = SM_MOD1();
    SmModule *pp = (SmModule *) p;

    SmModule::RegisterInterface(pp);

    SmDocShell::Factory().RegisterMenuBar( SmResId(RID_SMMENU) );
    SmDocShell::Factory().RegisterPluginMenuBar( SmResId(RID_SMPLUGINMENU) );
    SmDocShell::Factory().RegisterAccel ( SmResId(RID_SMACCEL) );
    SmDocShell::RegisterInterface(pp);

    SmViewShell::RegisterFactory(1);
    SmViewShell::RegisterInterface(pp);

    SvxZoomStatusBarControl::RegisterControl( SID_ATTR_ZOOM, pp );
    SvxModifyControl::RegisterControl( SID_TEXTSTATUS, pp );
    SvxUndoRedoControl::RegisterControl( SID_UNDO, pp );
    SvxUndoRedoControl::RegisterControl( SID_REDO, pp );

    SmToolBoxWrapper::RegisterChildWindow(TRUE);
    SmCmdBoxWrapper::RegisterChildWindow(TRUE);
}

/*************************************************************************
|*
|* Deinitialisierung
|*
\************************************************************************/
void SmDLL::Exit()
{
    // the SdModule must be destroyed
    SmModule** ppShlPtr = (SmModule**) GetAppData(SHL_SM);
    delete (*ppShlPtr);
    (*ppShlPtr) = NULL;

    *GetAppData(SHL_SM) = 0;
}

ULONG SmDLL::DetectFilter( SfxMedium& rMedium, const SfxFilter** ppFilter,
                            SfxFilterFlags nMust, SfxFilterFlags nDont )
{
    ULONG nReturn = ERRCODE_ABORT;
    if( SVSTREAM_OK != rMedium.GetError() )
        nReturn = rMedium.GetError();
    else if ( rMedium.IsStorage() )
    {
        // Storage
        SvStorage* pStorage = rMedium.GetStorage();

        if( !pStorage )
            nReturn = ULONG_MAX;
        else
        {
            // Erkennung ueber contained streams (StarChart 3.0)
            static const sal_Char sStrmNm_0[] = "StarMathDocument";
            static const sal_Char sFltrNm_0[] = "StarMath 5.0";
            static const sal_Char sStrmNm_1[] = "Equation Native";
            static const sal_Char sFltrNm_1[] = "MathType 3.x";
            static const sal_Char sStrmNm_2[] = "content.xml";
            static const sal_Char sFltrNm_2[] = STAROFFICE_XML;
            static const sal_Char sStrmNm_3[] = "Content.xml";
            static const sal_Char sFltrNm_3[] = STAROFFICE_XML;

            const sal_uInt16 nCount = 4;
            const sal_Char *aStrmNms[nCount] =
                { sStrmNm_0, sStrmNm_1, sStrmNm_2, sStrmNm_3 };
            const sal_Char *aFltrNms[nCount] =
                { sFltrNm_0, sFltrNm_1, sFltrNm_2, sFltrNm_3 };

            String aStreamName;
            String sFilterName;
            if( *ppFilter )
            {
                for( sal_uInt16 i=0; i < nCount; i++ )
                {
                    if( (*ppFilter)->GetFilterName().EqualsAscii(aFltrNms[i]) )
                    {
                        aStreamName.AssignAscii( aStrmNms[i] );
                        if( pStorage->IsStream( aStreamName ) &&
                            ((*ppFilter)->GetFilterFlags() & nMust) == nMust &&
                            ((*ppFilter)->GetFilterFlags() & nDont) == 0 )
                            nReturn = ERRCODE_NONE;

                        break;  // The old XML filter (Content.xml) will be
                                // detected in the next loop.
                    }
                }
            }

            if( ERRCODE_NONE != nReturn )
            {
                for( sal_uInt16 i=0; i < nCount; i++ )
                {
                    aStreamName.AssignAscii( aStrmNms[i] );
                    if( pStorage->IsStream( aStreamName ))
                    {
                        sFilterName.AssignAscii( aFltrNms[i] );
                        const SfxFilter* pFilt = SFX_APP()->GetFilter(
                                    SmDocShell::Factory(), sFilterName );

                        if( pFilt &&
                            (pFilt->GetFilterFlags() & nMust) == nMust &&
                            (pFilt->GetFilterFlags() & nDont) == 0)
                        {
                            *ppFilter = pFilt;
                            nReturn = ERRCODE_NONE;
                        }

                        break; // There are no two filters with the same strm name
                    }
                }
            }
        }
    }
    else
    {
        //Test to see if this begins with xml and if so run it through
        //the MathML filter. There are all sorts of things wrong with
        //this approach, to be fixed at a better level than here
        SvStream *pStrm = rMedium.GetInStream();
        if (pStrm && !pStrm->GetError())
        {
            const int nSize = 5;
            sal_Char aBuffer[nSize+1];
            aBuffer[nSize] = 0;
            ULONG nBytesRead = pStrm->Read( aBuffer, nSize );
            pStrm->Seek( STREAM_SEEK_TO_BEGIN );
            if (nBytesRead == nSize)
            {
                if (0 == strncmp( "<?xml",aBuffer,nSize))
                {
                    static const sal_Char sFltrNm_2[] = MATHML_XML;

                    String sFltrNm;
                    sFltrNm.AssignAscii( sFltrNm_2 );
                    const SfxFilter* pFilt = SFX_APP()->GetFilter(
                                    SmDocShell::Factory(), sFltrNm );
                    *ppFilter = pFilt;

                    nReturn = ERRCODE_NONE;
                }
            }
        }
    }
    return nReturn;
}

