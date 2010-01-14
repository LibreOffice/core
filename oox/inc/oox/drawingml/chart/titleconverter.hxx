/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: titleconverter.hxx,v $
 *
 * $Revision: 1.4 $
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

#ifndef OOX_DRAWINGML_CHART_TITLECONVERTER_HXX
#define OOX_DRAWINGML_CHART_TITLECONVERTER_HXX

#include "oox/drawingml/chart/converterbase.hxx"

namespace com { namespace sun { namespace star {
    namespace chart2 { class XDiagram; }
    namespace chart2 { class XFormattedString; }
    namespace chart2 { class XTitled; }
    namespace chart2 { namespace data { class XDataSequence; } }
} } }

namespace oox { namespace drawingml { struct TextCharacterProperties; } }

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct TextModel;

class TextConverter : public ConverterBase< TextModel >
{
public:
    explicit            TextConverter( const ConverterRoot& rParent, TextModel& rModel );
    virtual             ~TextConverter();

    /** Creates a data sequence object from the contained text data. */
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >
                        createDataSequence( const ::rtl::OUString& rRole );
    /** Creates a sequence of formatted string objects. */
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XFormattedString > >
                        createStringSequence(
                            const ::rtl::OUString& rDefaultText,
                            const ModelRef< TextBody >& rxTextProp,
                            ObjectType eObjType );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XFormattedString >
                        appendFormattedString(
                            ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XFormattedString > >& orStringVec,
                            const ::rtl::OUString& rString,
                            bool bAddNewLine ) const;
};

// ============================================================================

struct TitleModel;

class TitleConverter : public ConverterBase< TitleModel >
{
public:
    explicit            TitleConverter( const ConverterRoot& rParent, TitleModel& rModel );
    virtual             ~TitleConverter();

    /** Creates a title text object and attaches it at the passed interface. */
    void                convertFromModel(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTitled >& rxTitled,
                            const ::rtl::OUString& rAutoTitle, ObjectType eObjType,
                            sal_Int32 nMainIdx = -1, sal_Int32 nSubIdx = -1 );
};

// ============================================================================

struct LegendModel;

class LegendConverter : public ConverterBase< LegendModel >
{
public:
    explicit            LegendConverter( const ConverterRoot& rParent, LegendModel& rModel );
    virtual             ~LegendConverter();

    /** Creates a legend object and attaches it at the passed diagram. */
    void                convertFromModel(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >& rxDiagram );
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

