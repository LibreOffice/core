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

#include <oox/core/xmlfilterbase.hxx>

#include <com/sun/star/animations/XAnimationNode.hpp>
#include <oox/drawingml/theme.hxx>
#include <oox/ppt/presentationfragmenthandler.hxx>
#include <oox/ppt/slidepersist.hxx>
#include <vector>
#include <map>

namespace oox { namespace ppt {



class PowerPointImport : public oox::core::XmlFilterBase
{
public:

    PowerPointImport( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext )
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ~PowerPointImport();

    // from FilterBase
    virtual bool importDocument() override;
    virtual bool exportDocument() throw() override;

    virtual const ::oox::drawingml::Theme* getCurrentTheme() const override;
    virtual ::oox::vml::Drawing* getVmlDrawing() override;
    virtual const oox::drawingml::table::TableStyleListPtr getTableStyles() override;
    virtual ::oox::drawingml::chart::ChartConverter* getChartConverter() override;

    SlidePersistPtr                                         getActualSlidePersist() const { return mpActualSlidePersist; };
    void                                                    setActualSlidePersist( SlidePersistPtr pActualSlidePersist ){ mpActualSlidePersist = pActualSlidePersist; };
    std::map< OUString, oox::drawingml::ThemePtr >&    getThemes(){ return maThemes; };
    std::vector< SlidePersistPtr >&                         getDrawPages(){ return maDrawPages; };
    std::vector< SlidePersistPtr >&                         getMasterPages(){ return maMasterPages; };
    std::vector< SlidePersistPtr >&                         getNotesPages(){ return maNotesPages; };

    virtual sal_Bool SAL_CALL filter( const ::com::sun::star::uno::Sequence<   ::com::sun::star::beans::PropertyValue >& rDescriptor )
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    sal_Int32 getSchemeColor( sal_Int32 nToken ) const;

#if OSL_DEBUG_LEVEL > 0
    static XmlFilterBase* mpDebugFilterBase;
#endif

private:
    virtual GraphicHelper* implCreateGraphicHelper() const override;
    virtual ::oox::ole::VbaProject* implCreateVbaProject() const override;
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException, std::exception) override;

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

} }

#endif // INCLUDED_OOX_PPT_PPTIMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
