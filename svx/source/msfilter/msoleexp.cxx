/*************************************************************************
 *
 *  $RCSfile: msoleexp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:22 $
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

#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFX_INTERNO_HXX
#include <sfx2/interno.hxx>
#endif
#ifndef _SFX_OBJFAC_HXX
#include <sfx2/docfac.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX
#include <sfx2/fcontnr.hxx>
#endif

#include "msoleexp.hxx"

void SvxMSExportOLEObjects::ExportOLEObject( SvInPlaceObject& rObj,
                                                SvStorage& rDestStg )
{
    SfxInPlaceObjectRef xSfxIPObj( &rObj );
    const SfxFilter* pExpFilter = 0;
    if( xSfxIPObj.Is() && xSfxIPObj->GetObjectShell() && GetFlags() )
    {
        static struct _ObjExpType {
            UINT32 nFlag;
            const char* pFilterNm;
            // GlobalNameId
            struct _GlobalNameIds {
                UINT32 n1;
                USHORT n2, n3;
                BYTE b8, b9, b10, b11, b12, b13, b14, b15;
            } aGlNmIds[3];
        } aArr[] = {
            { OLE_STARMATH_2_MATHTYPE, "MathType 3.x",
                {SO3_SM_CLASSID_50, SO3_SM_CLASSID_40, SO3_SM_CLASSID_30 }},
            { OLE_STARWRITER_2_WINWORD, "MS Word 97",
                {SO3_SW_CLASSID_50, SO3_SW_CLASSID_40, SO3_SW_CLASSID_30 }},
            { OLE_STARCALC_2_EXCEL, "MS Excel 97",
                {SO3_SC_CLASSID_50, SO3_SC_CLASSID_40, SO3_SC_CLASSID_30 }},
            { OLE_STARIMPRESS_2_POWERPOINT, "MS PowerPoint 97",
                {SO3_SIMPRESS_CLASSID_50, SO3_SIMPRESS_CLASSID_40,
                 SO3_SIMPRESS_CLASSID_30 }},
            { 0,0 }
        };

        BOOL bFnd = FALSE;
        for( const _ObjExpType* pArr = aArr; !bFnd && pArr->nFlag; ++pArr )
            if( GetFlags() & pArr->nFlag )
            {
                for ( int n = 0; !bFnd && n < 3; ++n )
                {
                    const _ObjExpType::_GlobalNameIds& rId = pArr->aGlNmIds[ n ];
                    SvGlobalName aGlbNm( rId.n1, rId.n2, rId.n3,
                                rId.b8, rId.b9, rId.b10, rId.b11,
                                rId.b12, rId.b13, rId.b14, rId.b15 );
                    if( *xSfxIPObj->GetSvFactory() == aGlbNm )
                    {
                        const SfxObjectFactory& rFact = xSfxIPObj->
                                                GetObjectShell()->GetFactory();
                        if( rFact.GetFilterContainer() )
                            pExpFilter = rFact.GetFilterContainer()->
                                GetFilter4FilterName( String::CreateFromAscii(
                                                pArr->pFilterNm ));
                        bFnd = TRUE;
                    }
                }
            }
    }

    if( pExpFilter )                        // use this filter for the export
    {
        SfxMedium aMed( &rDestStg, FALSE );
        aMed.SetFilter( pExpFilter );
        xSfxIPObj->GetObjectShell()->ConvertTo( aMed );
    }
    else
    {
        rDestStg.SetVersion( SOFFICE_FILEFORMAT_31 );
        rObj.DoSaveAs( &rDestStg );
        rObj.DoSaveCompleted();
    }
}



