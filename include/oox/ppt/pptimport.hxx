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

#ifndef INCLUDED_OOX_PPT_PPTIMPORT_HXX
#define INCLUDED_OOX_PPT_PPTIMPORT_HXX

#include <map>
#include <memory>
#include <vector>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <oox/core/filterbase.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/drawingml/theme.hxx>
#include <oox/ppt/slidepersist.hxx>
#include <oox/ppt/pptshape.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace com::sun::star {
    namespace beans { struct PropertyValue; }
    namespace uno { class XComponentContext; }
}

namespace oox {
    class GraphicHelper;
    namespace drawingml::chart { class ChartConverter; }
    namespace ole { class VbaProject; }
    namespace vml { class Drawing; }
}

namespace oox::ppt {


class PowerPointImport final : public oox::core::XmlFilterBase
{
public:
    /// @throws css::uno::RuntimeException
    PowerPointImport( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~PowerPointImport() override;

    // from FilterBase
    virtual bool importDocument() override;
    virtual bool exportDocument() noexcept override;

    virtual const ::oox::drawingml::Theme* getCurrentTheme() const override;
    virtual ::oox::vml::Drawing* getVmlDrawing() override;
    virtual oox::drawingml::table::TableStyleListPtr getTableStyles() override;
    virtual ::oox::drawingml::chart::ChartConverter* getChartConverter() override;

    const SlidePersistPtr&                                  getActualSlidePersist() const { return mpActualSlidePersist; };
    void                                                    setActualSlidePersist( SlidePersistPtr pActualSlidePersist ){ mpActualSlidePersist = pActualSlidePersist; };
    std::map< OUString, oox::drawingml::ThemePtr >&         getThemes(){ return maThemes; };
    std::vector< SlidePersistPtr >&                         getDrawPages(){ return maDrawPages; };
    std::vector< SlidePersistPtr >&                         getMasterPages(){ return maMasterPages; };
    std::vector< SlidePersistPtr >&                         getNotesPages(){ return maNotesPages; };

    virtual sal_Bool SAL_CALL filter( const css::uno::Sequence<   css::beans::PropertyValue >& rDescriptor ) override;

    ::Color getSchemeColor( sal_Int32 nToken ) const;

    static std::vector< PPTShape* > maPPTShapes;

#if OSL_DEBUG_LEVEL > 0
    static XmlFilterBase* mpDebugFilterBase;
#endif

private:
    virtual GraphicHelper* implCreateGraphicHelper() const override;
    virtual ::oox::ole::VbaProject* implCreateVbaProject() const override;
    virtual OUString SAL_CALL getImplementationName() override;

private:
    OUString                                       maTableStyleListPath;
    oox::drawingml::table::TableStyleListPtr            mpTableStyleList;

    SlidePersistPtr                                     mpActualSlidePersist;
    std::map< OUString, oox::drawingml::ThemePtr > maThemes;

    std::vector< SlidePersistPtr > maDrawPages;
    std::vector< SlidePersistPtr > maMasterPages;
    std::vector< SlidePersistPtr > maNotesPages;

    std::shared_ptr< ::oox::drawingml::chart::ChartConverter > mxChartConv;
};

}

#endif // INCLUDED_OOX_PPT_PPTIMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
