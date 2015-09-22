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

#ifndef INCLUDED_SC_INC_APPOPTIO_HXX
#define INCLUDED_SC_INC_APPOPTIO_HXX

#include <vcl/field.hxx>
#include <sfx2/zoomitem.hxx>
#include <unotools/configitem.hxx>
#include "scdllapi.h"
#include "scmod.hxx"
#include "global.hxx"
#include "optutil.hxx"

class SC_DLLPUBLIC ScAppOptions
{
public:
                ScAppOptions();
                ScAppOptions( const ScAppOptions& rCpy );
                ~ScAppOptions();

    void        SetDefaults();

    void        SetAppMetric( FieldUnit eUnit ) { eMetric = eUnit;      }
    FieldUnit   GetAppMetric() const            { return eMetric;       }
    void        SetZoom( sal_uInt16 nNew )          { nZoom = nNew;         }
    sal_uInt16      GetZoom() const                 { return nZoom;         }
    void        SetZoomType( SvxZoomType eNew ) { eZoomType = eNew;     }
    SvxZoomType GetZoomType() const             { return eZoomType;     }
    void        SetSynchronizeZoom( bool bNew ) { bSynchronizeZoom = bNew; }
    bool        GetSynchronizeZoom() const      { return bSynchronizeZoom; }
    sal_uInt16      GetLRUFuncListCount() const     { return nLRUFuncCount; }
    sal_uInt16*     GetLRUFuncList() const          { return pLRUList;      }
    void        SetLRUFuncList( const sal_uInt16* pList,
                                const sal_uInt16  nCount );
    void        SetStatusFunc( sal_uInt16 nNew )    { nStatusFunc = nNew;   }
    sal_uInt16      GetStatusFunc() const           { return nStatusFunc;   }
    void        SetAutoComplete( bool bNew )    { bAutoComplete = bNew; }
    bool        GetAutoComplete() const         { return bAutoComplete; }
    void        SetDetectiveAuto( bool bNew )   { bDetectiveAuto = bNew; }
    bool        GetDetectiveAuto() const        { return bDetectiveAuto; }

    void        SetTrackContentColor(sal_uInt32 nNew) { nTrackContentColor = nNew; }
    sal_uInt32  GetTrackContentColor() const     { return nTrackContentColor; }
    void        SetTrackInsertColor(sal_uInt32 nNew)  { nTrackInsertColor = nNew;  }
    sal_uInt32  GetTrackInsertColor() const      { return nTrackInsertColor;  }
    void        SetTrackDeleteColor(sal_uInt32 nNew)  { nTrackDeleteColor = nNew;  }
    sal_uInt32  GetTrackDeleteColor() const      { return nTrackDeleteColor;  }
    void        SetTrackMoveColor(sal_uInt32 nNew)    { nTrackMoveColor = nNew;    }
    sal_uInt32  GetTrackMoveColor() const        { return nTrackMoveColor;    }

    ScLkUpdMode GetLinkMode() const             { return eLinkMode ;}
    void        SetLinkMode( ScLkUpdMode nSet ) {   eLinkMode  = nSet;}

    void        SetDefaultObjectSizeWidth(sal_Int32 nNew)   { nDefaultObjectSizeWidth = nNew; }
    sal_Int32       GetDefaultObjectSizeWidth() const       { return nDefaultObjectSizeWidth; }
    void        SetDefaultObjectSizeHeight(sal_Int32 nNew)  { nDefaultObjectSizeHeight = nNew; }
    sal_Int32       GetDefaultObjectSizeHeight() const      { return nDefaultObjectSizeHeight; }

    void        SetShowSharedDocumentWarning( bool bNew )   { mbShowSharedDocumentWarning = bNew; }
    bool        GetShowSharedDocumentWarning() const        { return mbShowSharedDocumentWarning; }
    ScOptionsUtil::KeyBindingType GetKeyBindingType() const { return meKeyBindingType; }
    void        SetKeyBindingType( ScOptionsUtil::KeyBindingType e ) { meKeyBindingType = e; }

    const ScAppOptions& operator=   ( const ScAppOptions& rOpt );

private:
    FieldUnit       eMetric;
    sal_uInt16      nLRUFuncCount;
    sal_uInt16*     pLRUList;
    SvxZoomType     eZoomType;
    sal_uInt16      nZoom;
    bool            bSynchronizeZoom;
    sal_uInt16      nStatusFunc;
    bool            bAutoComplete;
    bool            bDetectiveAuto;
    sal_uInt32  nTrackContentColor;
    sal_uInt32  nTrackInsertColor;
    sal_uInt32  nTrackDeleteColor;
    sal_uInt32  nTrackMoveColor;
    ScLkUpdMode eLinkMode;
    sal_Int32       nDefaultObjectSizeWidth;
    sal_Int32       nDefaultObjectSizeHeight;
    bool            mbShowSharedDocumentWarning;
    ScOptionsUtil::KeyBindingType meKeyBindingType;
};

//  Config Item containing app options

class ScAppCfg : public ScAppOptions
{
    //  spread about 5 config paths
    //! split ScAppOptions into different classes

    ScLinkConfigItem    aLayoutItem;
    ScLinkConfigItem    aInputItem;
    ScLinkConfigItem    aRevisionItem;
    ScLinkConfigItem    aContentItem;
    ScLinkConfigItem    aSortListItem;
    ScLinkConfigItem    aMiscItem;
    ScLinkConfigItem    aCompatItem;

    DECL_LINK_TYPED( LayoutCommitHdl, ScLinkConfigItem&, void );
    DECL_LINK_TYPED( InputCommitHdl, ScLinkConfigItem&, void );
    DECL_LINK_TYPED( RevisionCommitHdl, ScLinkConfigItem&, void );
    DECL_LINK_TYPED( ContentCommitHdl, ScLinkConfigItem&, void );
    DECL_LINK_TYPED( SortListCommitHdl, ScLinkConfigItem&, void );
    DECL_LINK_TYPED( MiscCommitHdl, ScLinkConfigItem&, void );
    DECL_LINK_TYPED( CompatCommitHdl, ScLinkConfigItem&, void );

    static com::sun::star::uno::Sequence<OUString> GetLayoutPropertyNames();
    static com::sun::star::uno::Sequence<OUString> GetInputPropertyNames();
    static com::sun::star::uno::Sequence<OUString> GetRevisionPropertyNames();
    static com::sun::star::uno::Sequence<OUString> GetContentPropertyNames();
    static com::sun::star::uno::Sequence<OUString> GetSortListPropertyNames();
    static com::sun::star::uno::Sequence<OUString> GetMiscPropertyNames();
    static com::sun::star::uno::Sequence<OUString> GetCompatPropertyNames();

public:
            ScAppCfg();

    void    SetOptions( const ScAppOptions& rNew );
    void    OptionsChanged();   // after direct access to ScAppOptions base class
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
