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

#ifndef INCLUDED_SVX_SVDOMEDIA_HXX
#define INCLUDED_SVX_SVDOMEDIA_HXX

#include <svx/svdorect.hxx>
#include <avmedia/mediaitem.hxx>
#include <svx/svxdllapi.h>

class Graphic;

namespace sdr { namespace contact { class ViewContactOfSdrMediaObj; } }


// - SdrMediaObj -


class SVX_DLLPUBLIC SdrMediaObj : public SdrRectObj
{
    friend class sdr::contact::ViewContactOfSdrMediaObj;

public:

                                    TYPEINFO_OVERRIDE();

                                    SdrMediaObj();
                                    SdrMediaObj( const Rectangle& rRect );

        virtual                     ~SdrMediaObj();

        virtual bool                HasTextEdit() const override;

        virtual void                TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
        virtual sal_uInt16              GetObjIdentifier() const override;

        virtual OUString            TakeObjNameSingul() const override;
        virtual OUString            TakeObjNamePlural() const override;

        virtual SdrMediaObj*            Clone() const override;
        SdrMediaObj&                operator=(const SdrMediaObj& rObj);

        virtual void                AdjustToMaxRect( const Rectangle& rMaxRect, bool bShrinkOnly = false ) override;

public:

        void                        setURL( const OUString& rURL, const OUString& rReferer, const OUString& rMimeType = OUString() );
        const OUString&      getURL() const;

        void                        setMediaProperties( const ::avmedia::MediaItem& rState );
        const ::avmedia::MediaItem& getMediaProperties() const;

        Size                        getPreferredSize() const;

        const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >
                                    getSnapshot() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream>
                                    GetInputStream();
        void                        SetInputStream(css::uno::Reference<css::io::XInputStream> const&);

protected:

        void                mediaPropertiesChanged( const ::avmedia::MediaItem& rNewState );
        virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() override;

private:
        struct Impl;
        std::unique_ptr<Impl> m_xImpl;
};

#endif // INCLUDED_SVX_SVDOMEDIA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
