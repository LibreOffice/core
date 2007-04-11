/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lnkbase.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:22:41 $
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
#ifndef _LNKBASE_HXX
#define _LNKBASE_HXX

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif
#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif
#ifndef _SOT_EXCHANGE_HXX //autogen
#include <sot/exchange.hxx>
#endif
#ifndef _TOOLS_REF_HXX
#include <tools/ref.hxx>
#endif
#ifndef _LINKSOURCE_HXX
#include <sfx2/linksrc.hxx>
#endif

namespace com { namespace sun { namespace star { namespace uno
{
    class Any;
    //class Type;
}}}}

namespace sfx2
{

struct ImplBaseLinkData;
class SvLinkManager;
class SvLinkSource;
class FileDialogHelper;

#ifndef OBJECT_DDE_EXTERN
#define OBJECT_INTERN       0x00
//#define   OBJECT_SO_EXTERN    0x01
#define OBJECT_DDE_EXTERN   0x02
#endif

#define OBJECT_CLIENT_SO            0x80 // ein Link
#define OBJECT_CLIENT_DDE           0x81
//#define   OBJECT_CLIENT_OLE           0x82 // ein Ole-Link
//#define   OBJECT_CLIENT_OLE_CACHE     0x83 // ein Ole-Link mit SvEmbeddedObject
#define OBJECT_CLIENT_FILE          0x90
#define OBJECT_CLIENT_GRF           0x91
#define OBJECT_CLIENT_OLE           0x92 // embedded link

enum sfxlink {
    // Ole2 compatibel und persistent
    LINKUPDATE_ALWAYS = 1,
    LINKUPDATE_ONCALL = 3,

    LINKUPDATE_END      // dummy!
};

struct BaseLink_Impl;

class SFX2_DLLPUBLIC SvBaseLink : public SvRefBase
{
private:
    friend class SvLinkManager;
    friend class SvLinkSource;

    SvLinkSourceRef         xObj;
    String                  aLinkName;
    BaseLink_Impl*          pImpl;
    USHORT                  nObjType;
    BOOL                    bVisible : 1;
    BOOL                    bSynchron : 1;
    BOOL                    bUseCache : 1;      // fuer GrafikLinks!
    BOOL                    bWasLastEditOK : 1;

    DECL_LINK( EndEditHdl, String* );

    bool                    ExecuteEdit( const String& _rNewName );

protected:
    void            SetObjType( USHORT );

                    // setzen des LinkSourceName ohne aktion
    void            SetName( const String & rLn );
                    // LinkSourceName der im SvLinkBase steht
    String          GetName() const;

    ImplBaseLinkData* pImplData;

    sal_Bool            m_bIsReadOnly;
    com::sun::star::uno::Reference<com::sun::star::io::XInputStream>
                        m_xInputStreamToLoadFrom;

                    SvBaseLink();
                    SvBaseLink( USHORT nLinkType, ULONG nContentType = FORMAT_STRING );
    virtual         ~SvBaseLink();

    void            _GetRealObject( BOOL bConnect = TRUE );

    SvLinkSource*   GetRealObject()
                    {
                        if( !xObj.Is() )
                            _GetRealObject();
                        return xObj;
                    }

public:
                    TYPEINFO();
                    // ask JP
    virtual void    Closed();
                    SvBaseLink( const String& rNm, USHORT nObjectType,
                                 SvLinkSource* );

    USHORT          GetObjType() const { return nObjType; }

    void            SetObj( SvLinkSource * pObj );
    SvLinkSource*   GetObj() const  { return xObj; }

    void            SetLinkSourceName( const String & rName );
    String          GetLinkSourceName() const;

    virtual void    DataChanged( const String & rMimeType,
                                const ::com::sun::star::uno::Any & rValue );

    void            SetUpdateMode( USHORT );
    USHORT          GetUpdateMode() const;
    ULONG           GetContentType() const;
    BOOL            SetContentType( ULONG nType );

    SvLinkManager*          GetLinkManager();
    const SvLinkManager*    GetLinkManager() const;
    void                    SetLinkManager( SvLinkManager* _pMgr );

    BOOL            Update();
    void            Disconnect();

    // Link impl: DECL_LINK( MyEndDialogHdl, SvBaseLink* ); <= param is this
    virtual void    Edit( Window*, const Link& rEndEditHdl );

        // soll der Link im Dialog angezeigt werden ? (Links im Link im ...)
    BOOL            IsVisible() const           { return bVisible; }
    void            SetVisible( BOOL bFlag )    { bVisible = bFlag; }
        // soll der Link synchron oder asynchron geladen werden?
    BOOL            IsSynchron() const          { return bSynchron; }
    void            SetSynchron( BOOL bFlag )   { bSynchron = bFlag; }

    BOOL            IsUseCache() const          { return bUseCache; }
    void            SetUseCache( BOOL bFlag )   { bUseCache = bFlag; }

    void            setStreamToLoadFrom(
                        const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& xInputStream,
                        sal_Bool bIsReadOnly )
                            { m_xInputStreamToLoadFrom = xInputStream;
                              m_bIsReadOnly = bIsReadOnly; }

    inline BOOL         WasLastEditOK() const       { return bWasLastEditOK; }
    FileDialogHelper*   GetFileDialog( sal_uInt32 nFlags, const String& rFactory ) const;
};

SV_DECL_IMPL_REF(SvBaseLink);

}

#endif
