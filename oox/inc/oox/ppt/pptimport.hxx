/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef OOX_POWERPOINT_POWERPOINTIMPORT_HXX
#define OOX_POWERPOINT_POWERPOINTIMPORT_HXX

#include "oox/core/xmlfilterbase.hxx"

#include <com/sun/star/animations/XAnimationNode.hpp>
#include <oox/drawingml/theme.hxx>
#include "oox/ppt/presentationfragmenthandler.hxx"
#include "oox/ppt/slidepersist.hxx"
#include "tokens.hxx"
#include <vector>
#include <map>

namespace oox { namespace ppt {

// ---------------------------------------------------------------------

class PowerPointImport : public oox::core::XmlFilterBase
{
public:

    PowerPointImport( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > & rSMgr  );
    virtual ~PowerPointImport();

    // from FilterBase
    virtual bool importDocument() throw();
    virtual bool exportDocument() throw();

    virtual const ::oox::drawingml::Theme* getCurrentTheme() const;
    virtual ::oox::vml::Drawing* getVmlDrawing();
    virtual const oox::drawingml::table::TableStyleListPtr getTableStyles();
    virtual ::oox::drawingml::chart::ChartConverter& getChartConverter();

    void													setActualSlidePersist( SlidePersistPtr pActualSlidePersist ){ mpActualSlidePersist = pActualSlidePersist; };
    std::map< rtl::OUString, oox::drawingml::ThemePtr >&	getThemes(){ return maThemes; };
    std::vector< SlidePersistPtr >&							getDrawPages(){ return maDrawPages; };
    std::vector< SlidePersistPtr >&							getMasterPages(){ return maMasterPages; };
    std::vector< SlidePersistPtr >&							getNotesPages(){ return maNotesPages; };

    virtual sal_Bool SAL_CALL filter( const ::com::sun::star::uno::Sequence<   ::com::sun::star::beans::PropertyValue >& rDescriptor )
        throw( ::com::sun::star::uno::RuntimeException );

    sal_Int32 getSchemeColor( sal_Int32 nToken ) const;

private:
    virtual GraphicHelper* implCreateGraphicHelper() const;
    virtual ::rtl::OUString implGetImplementationName() const;

private:
    rtl::OUString										maTableStyleListPath;
    oox::drawingml::table::TableStyleListPtr			mpTableStyleList;

    SlidePersistPtr										mpActualSlidePersist;
    std::map< rtl::OUString, oox::drawingml::ThemePtr > maThemes;

    std::vector< SlidePersistPtr > maDrawPages;
    std::vector< SlidePersistPtr > maMasterPages;
    std::vector< SlidePersistPtr > maNotesPages;

    ::boost::shared_ptr< ::oox::drawingml::chart::ChartConverter > mxChartConv;
};

} }

#endif // OOX_POWERPOINT_POWERPOINTIMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
