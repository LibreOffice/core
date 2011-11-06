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



#ifndef OOX_DRAWINGML_CHART_CHARTCONVERTER_HXX
#define OOX_DRAWINGML_CHART_CHARTCONVERTER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <oox/dllapi.h>

namespace com { namespace sun { namespace star {
    namespace awt { struct Point; }
    namespace awt { struct Size; }
    namespace drawing { class XShapes; }
    namespace chart2 { class XChartDocument; }
    namespace chart2 { namespace data { class XDataProvider; } }
    namespace chart2 { namespace data { class XDataSequence; } }
} } }

namespace oox { namespace core { class XmlFilterBase; } }

namespace oox {
namespace drawingml {
namespace chart {

struct ChartSpaceModel;
struct DataSequenceModel;

// ============================================================================

class OOX_DLLPUBLIC ChartConverter
{
public:
    explicit            ChartConverter();
    virtual             ~ChartConverter();

    /** Converts the passed OOXML chart model to the passed chart2 document.

        @param rChartModel  The filled MSOOXML chart model structure.

        @param rxChartDoc  The UNO chart document model to be initialized.

        @param rxExternalPage  If null, all embedded shapes will be inserted
            into the internal drawing page of the chart document. If not null,
            all embedded shapes will be inserted into this shapes collection.

        @param rChartPos  The position of the chart shape in its drawing page,
            in 1/100 mm. Will be used only, if parameter rxExternalPage is not
            null, for correct positioning of the embedded shapes in the
            external drawing page.

        @param rChartSize  The size of the chart shape in 1/100 mm. Needed for
            calculation of position and size of the chart elements (diagram,
            titles, legend, etc.) and embedded shapes.
     */
    void                convertFromModel(
                            ::oox::core::XmlFilterBase& rFilter,
                            ChartSpaceModel& rChartModel,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& rxChartDoc,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxExternalPage,
                            const ::com::sun::star::awt::Point& rChartPos,
                            const ::com::sun::star::awt::Size& rChartSize );

    /** Creates an internal data provider. Derived classes may override this
        function to create an external data provider. */
    virtual void        createDataProvider(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& rxChartDoc );

    /** Creates a data sequence from a formula. Dummy implementation. Derived
        classes have to override this function to actually parse the formula. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >
                        createDataSequence(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataProvider >& rxDataProvider,
                            const DataSequenceModel& rDataSeq );

private:
                        ChartConverter( const ChartConverter& );
    ChartConverter&     operator=( const ChartConverter& );
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif
