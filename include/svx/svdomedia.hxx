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

#include <memory>
#include <svx/svdorect.hxx>
#include <avmedia/mediaitem.hxx>
#include <svx/svxdllapi.h>

class Graphic;

namespace sdr::contact { class ViewContactOfSdrMediaObj; }
namespace com::sun::star::graphic { class XGraphic; }


class SVXCORE_DLLPUBLIC SdrMediaObj final : public SdrRectObj
{
    friend class sdr::contact::ViewContactOfSdrMediaObj;

private:
    // protected destructor - due to final, make private
    virtual ~SdrMediaObj() override;

public:
        SdrMediaObj(SdrModel& rSdrModel);
        // Copy constructor
        SdrMediaObj(SdrModel& rSdrModel, SdrMediaObj const & rSource);
        SdrMediaObj(
                SdrModel& rSdrModel,
                const tools::Rectangle& rRect);

        virtual bool                HasTextEdit() const override;

        virtual void                TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
        virtual SdrObjKind          GetObjIdentifier() const override;

        virtual OUString            TakeObjNameSingul() const override;
        virtual OUString            TakeObjNamePlural() const override;

        virtual SdrMediaObj*        CloneSdrObject(SdrModel& rTargetModel) const override;

        virtual void                AdjustToMaxRect( const tools::Rectangle& rMaxRect, bool bShrinkOnly = false ) override;

        void                        setURL( const OUString& rURL, const OUString& rReferer, const OUString& rMimeType = OUString() );
        const OUString&      getURL() const;

        void                        setMediaProperties( const ::avmedia::MediaItem& rState );
        const ::avmedia::MediaItem& getMediaProperties() const;

        css::uno::Reference< css::graphic::XGraphic > const &
                                    getSnapshot() const;
        css::uno::Reference< css::io::XInputStream>
                                    GetInputStream() const;
        void                        SetInputStream(css::uno::Reference<css::io::XInputStream> const&);

        virtual bool shouldKeepAspectRatio() const override { return true; }

private:
        void                mediaPropertiesChanged( const ::avmedia::MediaItem& rNewState );
        virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;

        struct Impl;
        std::unique_ptr<Impl> m_xImpl;
};

#endif // INCLUDED_SVX_SVDOMEDIA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
