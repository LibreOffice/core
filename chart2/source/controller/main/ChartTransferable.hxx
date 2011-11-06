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


#ifndef CHART2_CHARTTRANSFERABLE_HXX
#define CHART2_CHARTTRANSFERABLE_HXX

#include <svtools/transfer.hxx>

namespace com { namespace sun { namespace star {
namespace graphic {
    class XGraphic;
}
}}}

class SdrModel;
class SdrObject;

namespace chart
{

class ChartTransferable : public TransferableHelper
{
public:
    explicit ChartTransferable( SdrModel* pDrawModel, SdrObject* pSelectedObj, bool bDrawing = false );
    virtual ~ChartTransferable();

protected:

    // implementation of TransferableHelper methods
    virtual void        AddSupportedFormats();
    virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    virtual sal_Bool    WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId,
                                        const ::com::sun::star::datatransfer::DataFlavor& rFlavor );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > m_xMetaFileGraphic;
    SdrModel* m_pMarkedObjModel;
    bool m_bDrawing;
};

} //  namespace chart

// CHART2_CHARTTRANSFERABLE_HXX
#endif
