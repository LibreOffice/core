/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
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
#ifndef _LINKSRC_HXX
#define _LINKSRC_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

#ifndef _TOOLS_REF_HXX
#include <tools/ref.hxx>
#endif
#include <tools/rtti.hxx>
#include <com/sun/star/io/XInputStream.hpp>

#ifndef _SAL_CONFIG_H
#include <sal/config.h>
#define _SAL_CONFIG_H
#endif

namespace com { namespace sun { namespace star { namespace uno
{
    class Any;
    //class Type;
}}}}
class Window;
class String;

#ifndef ADVISEMODE_NODATA
// Muessen die gleichen Werte wie Ole2 ADVF_* sein
#define ADVISEMODE_NODATA       0x01
//#define ADVISEMODE_PRIMEFIRST   0x02
#define ADVISEMODE_ONLYONCE     0x04
//#define ADVISEMODE_DATAONSTOP   0x40
#endif

namespace sfx2
{

class SvBaseLink;
struct SvLinkSource_Impl;

class SFX2_DLLPUBLIC SvLinkSource : public SvRefBase
{
private:
    SvLinkSource_Impl*  pImpl; // compatible area

public:
                        TYPEINFO();

                        SvLinkSource();
    virtual             ~SvLinkSource();

//  SvBaseLink*         GetDataBaseLink() const;
    sal_Bool                HasDataLinks( const SvBaseLink* = 0 ) const;

    void                Closed();

    sal_uIntPtr                 GetUpdateTimeout() const;
    void                SetUpdateTimeout( sal_uIntPtr nTime );
                        // notify the sink, the mime type is not
                        // a selection criterion
    void                DataChanged( const String & rMimeType,
                                    const ::com::sun::star::uno::Any & rVal );
    void                SendDataChanged();
    void                NotifyDataChanged();

    virtual sal_Bool        Connect( SvBaseLink* );
    virtual sal_Bool        GetData( ::com::sun::star::uno::Any & rData /*out param*/,
                                const String & rMimeType,
                                sal_Bool bSynchron = sal_False );

                        // sal_True => waitinmg for data
    virtual sal_Bool        IsPending() const;
                        // sal_True => data complete loaded
    virtual sal_Bool        IsDataComplete() const;

    // Link impl: DECL_LINK( MyEndEditHdl, sfx2::FileDialogHelper* ); <= param is the dialog
    virtual void        Edit( Window *, SvBaseLink *, const Link& rEndEditHdl );


    void                AddDataAdvise( SvBaseLink *, const String & rMimeType,
                                        sal_uInt16 nAdviceMode );
    void                RemoveAllDataAdvise( SvBaseLink * );

    void                AddConnectAdvise( SvBaseLink * );
    void                RemoveConnectAdvise( SvBaseLink * );

    struct StreamToLoadFrom{
        StreamToLoadFrom(
            const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& xInputStream,sal_Bool bIsReadOnly )
            :m_xInputStreamToLoadFrom(xInputStream),
             m_bIsReadOnly(bIsReadOnly)
        {
        }

        com::sun::star::uno::Reference<com::sun::star::io::XInputStream>
        m_xInputStreamToLoadFrom;
        sal_Bool m_bIsReadOnly;
    };

    StreamToLoadFrom getStreamToLoadFrom();
    void setStreamToLoadFrom(const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& xInputStream,sal_Bool bIsReadOnly );
    // --> OD 2008-06-18 #i88291#
    void clearStreamToLoadFrom();
    // <--
};

SV_DECL_IMPL_REF(SvLinkSource);

}
#endif
