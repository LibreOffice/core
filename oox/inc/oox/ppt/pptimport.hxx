/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef OOX_POWERPOINT_POWERPOINTIMPORT_HXX
#define OOX_POWERPOINT_POWERPOINTIMPORT_HXX

#include "oox/core/xmlfilterbase.hxx"

#include <com/sun/star/animations/XAnimationNode.hpp>
#include <oox/drawingml/theme.hxx>
#include "oox/ppt/presentationfragmenthandler.hxx"
#include "oox/ppt/slidepersist.hxx"
#include <vector>
#include <map>

namespace oox { namespace ppt {

// ---------------------------------------------------------------------

class PowerPointImport : public oox::core::XmlFilterBase
{
public:

    PowerPointImport( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext )
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ~PowerPointImport();

    // from FilterBase
    virtual bool importDocument() throw();
    virtual bool exportDocument() throw();

    virtual const ::oox::drawingml::Theme* getCurrentTheme() const;
    virtual ::oox::vml::Drawing* getVmlDrawing();
    virtual const oox::drawingml::table::TableStyleListPtr getTableStyles();
    virtual ::oox::drawingml::chart::ChartConverter& getChartConverter();

    void                                                    setActualSlidePersist( SlidePersistPtr pActualSlidePersist ){ mpActualSlidePersist = pActualSlidePersist; };
    std::map< rtl::OUString, oox::drawingml::ThemePtr >&    getThemes(){ return maThemes; };
    std::vector< SlidePersistPtr >&                         getDrawPages(){ return maDrawPages; };
    std::vector< SlidePersistPtr >&                         getMasterPages(){ return maMasterPages; };
    std::vector< SlidePersistPtr >&                         getNotesPages(){ return maNotesPages; };

    sal_Int32 getSchemeColor( sal_Int32 nToken ) const;

private:
    virtual GraphicHelper* implCreateGraphicHelper() const;
    virtual ::oox::ole::VbaProject* implCreateVbaProject() const;
    virtual ::rtl::OUString implGetImplementationName() const;

private:
    rtl::OUString                                       maTableStyleListPath;
    oox::drawingml::table::TableStyleListPtr            mpTableStyleList;

    SlidePersistPtr                                     mpActualSlidePersist;
    std::map< rtl::OUString, oox::drawingml::ThemePtr > maThemes;

    std::vector< SlidePersistPtr > maDrawPages;
    std::vector< SlidePersistPtr > maMasterPages;
    std::vector< SlidePersistPtr > maNotesPages;

    ::boost::shared_ptr< ::oox::drawingml::chart::ChartConverter > mxChartConv;
};

} }

#endif // OOX_POWERPOINT_POWERPOINTIMPORT_HXX
