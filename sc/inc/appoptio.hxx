/*************************************************************************
 *
 *  $RCSfile: appoptio.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-02 19:10:56 $
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

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef SC_OPTUTIL_HXX
#include "optutil.hxx"
#endif


class ScAppOptions
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

    void        SetTrackContentColor(ULONG nNew) { nTrackContentColor = nNew; }
    ULONG       GetTrackContentColor() const     { return nTrackContentColor; }
    void        SetTrackInsertColor(ULONG nNew)  { nTrackInsertColor = nNew;  }
    ULONG       GetTrackInsertColor() const      { return nTrackInsertColor;  }
    void        SetTrackDeleteColor(ULONG nNew)  { nTrackDeleteColor = nNew;  }
    ULONG       GetTrackDeleteColor() const      { return nTrackDeleteColor;  }
    void        SetTrackMoveColor(ULONG nNew)    { nTrackMoveColor = nNew;    }
    ULONG       GetTrackMoveColor() const        { return nTrackMoveColor;    }

    ScLkUpdMode GetLinkMode() const             { return eLinkMode ;}
    void        SetLinkMode( ScLkUpdMode nSet ) {   eLinkMode  = nSet;}


    const ScAppOptions& operator=   ( const ScAppOptions& rOpt );
    friend SvStream&    operator>>  ( SvStream& rStream, ScAppOptions& rOpt );
    friend SvStream&    operator<<  ( SvStream& rStream, const ScAppOptions& rOpt );

private:
    FieldUnit   eMetric;
    USHORT      nLRUFuncCount;
    USHORT*     pLRUList;
    SvxZoomType eZoomType;
    USHORT      nZoom;
    USHORT      nStatusFunc;
    BOOL        bAutoComplete;
    BOOL        bDetectiveAuto;
    ULONG       nTrackContentColor;
    ULONG       nTrackInsertColor;
    ULONG       nTrackDeleteColor;
    ULONG       nTrackMoveColor;
    ScLkUpdMode eLinkMode;
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

    DECL_LINK( LayoutCommitHdl, void* );
    DECL_LINK( InputCommitHdl, void* );
    DECL_LINK( RevisionCommitHdl, void* );
    DECL_LINK( ContentCommitHdl, void* );
    DECL_LINK( SortListCommitHdl, void* );

    com::sun::star::uno::Sequence<rtl::OUString> GetLayoutPropertyNames();
    com::sun::star::uno::Sequence<rtl::OUString> GetInputPropertyNames();
    com::sun::star::uno::Sequence<rtl::OUString> GetRevisionPropertyNames();
    com::sun::star::uno::Sequence<rtl::OUString> GetContentPropertyNames();
    com::sun::star::uno::Sequence<rtl::OUString> GetSortListPropertyNames();

public:
            ScAppCfg();

    void    SetOptions( const ScAppOptions& rNew );
    void    OptionsChanged();   // after direct access to ScAppOptions base class
};


#endif

