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



#ifndef _SD_SDFILTER_HXX
#define _SD_SDFILTER_HXX

#include <tools/gen.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>

// ------------
// - SdFilter -
// ------------

class SfxMedium;
namespace sd {
class DrawDocShell;
}
class SdDrawDocument;
class SfxProgress;
namespace osl { class Module; }

class SdFilter
{
public:
    SdFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell, sal_Bool bShowProgress );
    virtual ~SdFilter();

    sal_Bool                    IsProgress() const { return mbShowProgress; }
    sal_Bool                    IsDraw() const { return mbIsDraw; }
    sal_Bool                    IsImpress() const { return !mbIsDraw; }
    virtual sal_Bool            Export() = 0;

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >             mxModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator >    mxStatusIndicator;

    SfxMedium&                  mrMedium;
    ::sd::DrawDocShell&         mrDocShell;
    SdDrawDocument&             mrDocument;
    sal_Bool                    mbIsDraw : 1;
    sal_Bool                    mbShowProgress : 1;

    ::osl::Module*              OpenLibrary( const ::rtl::OUString& rLibraryName ) const;
    void                        CreateStatusIndicator();

private:

    ::rtl::OUString             ImplGetFullLibraryName( const ::rtl::OUString& rLibraryName ) const;

};

#endif // _SD_SDFILTER_HXX
