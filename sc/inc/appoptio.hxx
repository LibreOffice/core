/*************************************************************************
 *
 *  $RCSfile: appoptio.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:47 $
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

#ifndef _SFXCFGITEM_HXX //autogen
#include <sfx2/cfgitem.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
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
// CfgItem fuer App-Optionen
//==================================================================

class ScAppCfg : public ScAppOptions,
                 public SfxConfigItem
{
public:
            ScAppCfg();

    virtual String GetName() const;

protected:
    virtual int     Load        (SvStream& rStream);
    virtual BOOL    Store       (SvStream& rStream);
    virtual void    UseDefault  ();
};


#endif

