/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmmodel.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:51:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVX_FMMODEL_HXX
#define _SVX_FMMODEL_HXX

#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SfxObjectShell;
class SfxItemPool;
class VCItemPool;
class FmXUndoEnvironment;
class SfxObjectShell;
class SbxObject;
class SbxArray;
class SbxValue;

struct FmFormModelImplData;
class SVX_DLLPUBLIC FmFormModel :
    public SdrModel
{
private:
    FmFormModelImplData*    m_pImpl;
    SfxObjectShell*         m_pObjShell;

    sal_Bool            m_bOpenInDesignMode : 1;
    sal_Bool            m_bAutoControlFocus : 1;

    SVX_DLLPRIVATE FmFormModel( const FmFormModel& );   // never implemented
    SVX_DLLPRIVATE void operator=(const FmFormModel& rSrcModel);   // never implemented

public:
    TYPEINFO();

    FmFormModel(SfxItemPool* pPool=NULL, SfxObjectShell* pPers=NULL );
    FmFormModel(const XubString& rPath, SfxItemPool* pPool=NULL,
                SfxObjectShell* pPers=NULL );
    FmFormModel(SfxItemPool* pPool, SfxObjectShell* pPers, FASTBOOL bUseExtColorTable);
    FmFormModel(const XubString& rPath, SfxItemPool* pPool, SfxObjectShell* pPers,
                FASTBOOL bUseExtColorTable);

    virtual ~FmFormModel();

    virtual SdrPage* AllocPage(FASTBOOL bMasterPage);
    virtual void     InsertPage(SdrPage* pPage, sal_uInt16 nPos=0xFFFF);
    virtual SdrPage* RemovePage(sal_uInt16 nPgNum);
    virtual void     MovePage(USHORT nPgNum, USHORT nNewPos);
    virtual void     InsertMasterPage(SdrPage* pPage, sal_uInt16 nPos=0xFFFF);
    virtual SdrPage* RemoveMasterPage(sal_uInt16 nPgNum);

    virtual SdrLayerID      GetControlExportLayerId( const SdrObject& rObj ) const;
    SfxObjectShell*         GetObjectShell() const { return m_pObjShell; }
    void                    SetObjectShell( SfxObjectShell* pShell );

    sal_Bool GetOpenInDesignMode() const { return m_bOpenInDesignMode; }
    void SetOpenInDesignMode( sal_Bool _bOpenDesignMode );

    sal_Bool    GetAutoControlFocus() const { return m_bAutoControlFocus; }
    void        SetAutoControlFocus( sal_Bool _bAutoControlFocus );

    /** check whether the OpenInDesignMode has been set explicitly or been loaded (<FALSE/>)
        or if it still has the default value from construction (<TRUE/>)
    */
    sal_Bool    OpenInDesignModeIsDefaulted( );

//#if 0 // _SOLAR__PRIVATE
    void        implSetOpenInDesignMode( sal_Bool _bOpenDesignMode, sal_Bool _bForce );

    FmXUndoEnvironment& GetUndoEnv();
//#endif

};

#endif          // _FM_FMMODEL_HXX

