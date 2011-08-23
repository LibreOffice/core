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

#ifndef _SD_SDFILTER_HXX
#define _SD_SDFILTER_HXX

#include "rtl/ustring.hxx"

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORSUPPLIER_HPP_
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#endif
namespace vos { class OModule; }
namespace binfilter {

// ------------
// - SdFilter -
// ------------

class SfxMedium;
class SdDrawDocShell;
class SdDrawDocument;
class SfxProgress;

class SdFilter
{
protected:

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >				mxModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator >	mxStatusIndicator;

protected:

    SfxMedium&					mrMedium;
    SdDrawDocShell&				mrDocShell;
    SdDrawDocument&				mrDocument;
    SfxProgress*				mpProgress;
    sal_Bool					mbIsDraw : 1;
    sal_Bool					mbShowProgress : 1;

    void						CreateStatusIndicator();
    void						CreateProgress();

public:
    
                                SdFilter( SfxMedium& rMedium, SdDrawDocShell& rDocShell, sal_Bool bShowProgress );
                                ~SdFilter();

    virtual sal_Bool			Import();
    virtual sal_Bool			Export();

    sal_Bool					IsProgress() const { return mbShowProgress; }
    sal_Bool					IsDraw() const { return mbIsDraw; }
    sal_Bool					IsImpress() const { return !mbIsDraw; }
};

} //namespace binfilter
#endif // _SD_SDFILTER_HXX
