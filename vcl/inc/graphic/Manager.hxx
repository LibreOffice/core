/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_GRAPHIC_MANAGER_HXX
#define INCLUDED_VCL_INC_GRAPHIC_MANAGER_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/animate.hxx>
#include <vcl/vectorgraphicdata.hxx>
#include <vcl/metaact.hxx>
#include <vcl/timer.hxx>
#include <vcl/GraphicExternalLink.hxx>

#include <officecfg/Office/Common.hxx>
#include <unotools/configmgr.hxx>

#include <memory>
#include <chrono>
#include <unordered_set>

class ImpGraphic;

namespace vcl
{
namespace graphic
{
class Manager final
{
private:
    std::unordered_set<ImpGraphic*> m_pImpGraphicList;
    std::chrono::seconds mnAllowedIdleTime;
    sal_Int64 mnMemoryLimit;
    sal_Int64 mnUsedSize;
    Timer maSwapOutTimer;

    Manager();

    void registerGraphic(const std::shared_ptr<ImpGraphic>& rImpGraphic, OUString const& rsContext);

    DECL_LINK(SwapOutTimerHandler, Timer*, void);

    static sal_Int64 getGraphicSizeBytes(const ImpGraphic* pImpGraphic);

public:
    static Manager& get();

    void swappedIn(const ImpGraphic* pImpGraphic);
    void swappedOut(const ImpGraphic* pImpGraphic);

    void reduceGraphicMemory();
    void changeExisting(const ImpGraphic* pImpGraphic, sal_Int64 nOldSize);
    void unregisterGraphic(ImpGraphic* pImpGraphic);

    std::shared_ptr<ImpGraphic> copy(std::shared_ptr<ImpGraphic> const& pImpGraphic);
    std::shared_ptr<ImpGraphic> newInstance();
    std::shared_ptr<ImpGraphic> newInstance(const Bitmap& rBitmap);
    std::shared_ptr<ImpGraphic> newInstance(const BitmapEx& rBitmapEx);
    std::shared_ptr<ImpGraphic> newInstance(const VectorGraphicDataPtr& rVectorGraphicDataPtr);
    std::shared_ptr<ImpGraphic> newInstance(const Animation& rAnimation);
    std::shared_ptr<ImpGraphic> newInstance(const GDIMetaFile& rMtf);
    std::shared_ptr<ImpGraphic> newInstance(const GraphicExternalLink& rGraphicLink);
};
}
} // end namespace vcl::graphic

#endif // INCLUDED_VCL_INC_GRAPHIC_MANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
