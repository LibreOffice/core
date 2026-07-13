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

#pragma once

#include <memory>
#include <svx/svdorect.hxx>
#include <avmedia/mediaitem.hxx>
#include <svx/svxdllapi.h>

namespace sdr::contact { class ViewContactOfSdrMediaObj; }
namespace com::sun::star::graphic { class XGraphic; }

class SdrMediaLink;

class SVXCORE_DLLPUBLIC SdrMediaObj final : public SdrRectObj
{
    friend class sdr::contact::ViewContactOfSdrMediaObj;
    friend class SdrMediaLink;

private:
    // protected destructor - due to final, make private
    SAL_DLLPRIVATE virtual ~SdrMediaObj() override;

public:
        SAL_DLLPRIVATE SdrMediaObj(SdrModel& rSdrModel);
        // Copy constructor
        SAL_DLLPRIVATE SdrMediaObj(SdrModel& rSdrModel, SdrMediaObj const & rSource);
        SdrMediaObj(
                SdrModel& rSdrModel,
                const tools::Rectangle& rRect);

        SAL_DLLPRIVATE virtual bool                HasTextEdit() const override;

        SAL_DLLPRIVATE virtual void                TakeObjInfo(SdrObjTransformInfoRec& rInfo) const override;
        SAL_DLLPRIVATE virtual SdrObjKind          GetObjIdentifier() const override;

        SAL_DLLPRIVATE virtual OUString            TakeObjNameSingul() const override;
        SAL_DLLPRIVATE virtual OUString            TakeObjNamePlural() const override;

        virtual rtl::Reference<SdrObject> CloneSdrObject(SdrModel& rTargetModel) const override;

        virtual void                AdjustToMaxRect( const tools::Rectangle& rMaxRect, bool bShrinkOnly = false ) override;

        void                        setURL(const OUString& rURL, const OUString& rReferer);
        const OUString&      getURL() const;

        /// Returns the URL to the temporary extracted media file.
        const OUString&      getTempURL() const;

        SAL_DLLPRIVATE void                        setMediaProperties( const ::avmedia::MediaItem& rState );
        const ::avmedia::MediaItem& getMediaProperties() const;

        css::uno::Reference< css::graphic::XGraphic > const &
                                    getSnapshot() const;
        css::uno::Reference< css::io::XInputStream>
                                    GetInputStream() const;
        SAL_DLLPRIVATE void                        SetInputStream(css::uno::Reference<css::io::XInputStream> const&);

        virtual bool shouldKeepAspectRatio() const override { return true; }

private:
        SAL_DLLPRIVATE void                mediaPropertiesChanged( const ::avmedia::MediaItem& rNewState );
        SAL_DLLPRIVATE virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;
        SAL_DLLPRIVATE virtual void        handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage) override;

        // Register an external media reference as a document link, so it takes
        // part in the normal link update permission just like a linked graphic.
        // Media stored inside the document is not a link and is left alone.
        SAL_DLLPRIVATE void                ImpRegisterLink();
        SAL_DLLPRIVATE void                ImpDeregisterLink();

        // Fetch a snapshot frame from the media URL. The caller decides
        // whether a fetch is allowed.
        SAL_DLLPRIVATE void                grabSnapshot(const OUString& rRealURL) const;

        struct Impl;
        std::unique_ptr<Impl> m_xImpl;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
