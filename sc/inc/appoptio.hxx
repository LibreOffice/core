/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: appoptio.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 12:14:31 $
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

#ifndef SC_APPOPTIO_HXX
#define SC_APPOPTIO_HXX

#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _SVX_ZOOMITEM_HXX //autogen
#include <svx/zoomitem.hxx>
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef INCLUDED_SCDLLAPI_H
#include "scdllapi.h"
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef SC_OPTUTIL_HXX
#include "optutil.hxx"
#endif

class SC_DLLPUBLIC ScAppOptions
{
public:
                ScAppOptions();
                ScAppOptions( const ScAppOptions& rCpy );
                ~ScAppOptions();

    void        SetDefaults();

    void        SetAppMetric( FieldUnit eUnit ) { eMetric = eUnit;      }
    FieldUnit   GetAppMetric() const            { return eMetric;       }
    void        SetZoom( USHORT nNew )          { nZoom = nNew;         }
    USHORT      GetZoom() const                 { return nZoom;         }
    void        SetZoomType( SvxZoomType eNew ) { eZoomType = eNew;     }
    SvxZoomType GetZoomType() const             { return eZoomType;     }
    void        SetSynchronizeZoom( BOOL bNew ) { bSynchronizeZoom = bNew; }
    BOOL        GetSynchronizeZoom() const      { return bSynchronizeZoom; }
    USHORT      GetLRUFuncListCount() const     { return nLRUFuncCount; }
    USHORT*     GetLRUFuncList() const          { return pLRUList;      }
    void        SetLRUFuncList( const USHORT* pList,
                                const USHORT  nCount );
    void        SetStatusFunc( USHORT nNew )    { nStatusFunc = nNew;   }
    USHORT      GetStatusFunc() const           { return nStatusFunc;   }
    void        SetAutoComplete( BOOL bNew )    { bAutoComplete = bNew; }
    BOOL        GetAutoComplete() const         { return bAutoComplete; }
    void        SetDetectiveAuto( BOOL bNew )   { bDetectiveAuto = bNew; }
    BOOL        GetDetectiveAuto() const        { return bDetectiveAuto; }

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

    void        SetDefaultObjectSizeWidth(INT32 nNew)   { nDefaultObjectSizeWidth = nNew; }
    INT32       GetDefaultObjectSizeWidth() const       { return nDefaultObjectSizeWidth; }
    void        SetDefaultObjectSizeHeight(INT32 nNew)  { nDefaultObjectSizeHeight = nNew; }
    INT32       GetDefaultObjectSizeHeight() const      { return nDefaultObjectSizeHeight; }

    void        SetShowSharedDocumentWarning( BOOL bNew )   { mbShowSharedDocumentWarning = bNew; }
    BOOL        GetShowSharedDocumentWarning() const        { return mbShowSharedDocumentWarning; }


    const ScAppOptions& operator=   ( const ScAppOptions& rOpt );
    friend SvStream&    operator>>  ( SvStream& rStream, ScAppOptions& rOpt );
    friend SvStream&    operator<<  ( SvStream& rStream, const ScAppOptions& rOpt );

private:
    FieldUnit   eMetric;
    USHORT      nLRUFuncCount;
    USHORT*     pLRUList;
    SvxZoomType eZoomType;
    USHORT      nZoom;
    BOOL        bSynchronizeZoom;
    USHORT      nStatusFunc;
    BOOL        bAutoComplete;
    BOOL        bDetectiveAuto;
    sal_uInt32  nTrackContentColor;
    sal_uInt32  nTrackInsertColor;
    sal_uInt32  nTrackDeleteColor;
    sal_uInt32  nTrackMoveColor;
    ScLkUpdMode eLinkMode;
    INT32       nDefaultObjectSizeWidth;
    INT32       nDefaultObjectSizeHeight;
    BOOL        mbShowSharedDocumentWarning;
};


//==================================================================
//  Config Item containing app options
//==================================================================

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

    DECL_LINK( LayoutCommitHdl, void* );
    DECL_LINK( InputCommitHdl, void* );
    DECL_LINK( RevisionCommitHdl, void* );
    DECL_LINK( ContentCommitHdl, void* );
    DECL_LINK( SortListCommitHdl, void* );
    DECL_LINK( MiscCommitHdl, void* );

    com::sun::star::uno::Sequence<rtl::OUString> GetLayoutPropertyNames();
    com::sun::star::uno::Sequence<rtl::OUString> GetInputPropertyNames();
    com::sun::star::uno::Sequence<rtl::OUString> GetRevisionPropertyNames();
    com::sun::star::uno::Sequence<rtl::OUString> GetContentPropertyNames();
    com::sun::star::uno::Sequence<rtl::OUString> GetSortListPropertyNames();
    com::sun::star::uno::Sequence<rtl::OUString> GetMiscPropertyNames();

public:
            ScAppCfg();

    void    SetOptions( const ScAppOptions& rNew );
    void    OptionsChanged();   // after direct access to ScAppOptions base class
};


#endif

