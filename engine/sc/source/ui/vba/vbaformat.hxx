/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#pragma once

#include <com/sun/star/lang/Locale.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <rtl/ref.hxx>

namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::beans { class XPropertyState; }
namespace com::sun::star::frame { class XModel; }
namespace com::sun::star::util { class XNumberFormats; }
namespace com::sun::star::util { class XNumberFormatsSupplier; }
namespace com::sun::star::util { class XNumberFormatTypes; }
namespace ooo::vba::excel { class XFont; }
namespace ooo::vba::excel { class XInterior; }

class ScCellRangesBase;
class ScModelObj;
class SfxItemSet;

template< typename... Ifc >
class ScVbaFormat : public InheritedHelperInterfaceWeakImpl< Ifc... >
{
typedef InheritedHelperInterfaceWeakImpl< Ifc... > ScVbaFormat_BASE;
    css::lang::Locale m_aDefaultLocale;
protected:
    cpo::uno::Reference< css::beans::XPropertySet > mxPropertySet;
    cpo::uno::Reference< css::util::XNumberFormatsSupplier > mxNumberFormatsSupplier;
    cpo::uno::Reference< css::util::XNumberFormats > xNumberFormats;
    cpo::uno::Reference< css::util::XNumberFormatTypes > xNumberFormatTypes;
    rtl::Reference< ScModelObj > mxModel;
    cpo::uno::Reference< css::beans::XPropertyState > xPropertyState;
    bool mbCheckAmbiguoity;
    bool mbAddIndent;
    /// @throws css::script::BasicErrorException
    bool isAmbiguous(const OUString& _sPropertyName);
    /// @throws cpo::uno::RuntimeException
    cpo::uno::Reference< css::beans::XPropertyState > const &  getXPropertyState();
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    void initializeNumberFormats();
    /// @throws cpo::uno::RuntimeException
    SfxItemSet*  getCurrentDataSet( );
protected:
    /// @throws cpo::uno::RuntimeException
    virtual ScCellRangesBase* getCellRangesBase();
public:
    /// @throws css::script::BasicErrorException
    ScVbaFormat( const cpo::uno::Reference< ov::XHelperInterface >& xParent, const cpo::uno::Reference< cpo::uno::XComponentContext > & xContext, cpo::uno::Reference< css::beans::XPropertySet >  _xPropertySet, const rtl::Reference<ScModelObj>& xModel, bool bCheckAmbiguoity );
    virtual cpo::uno::Reference< ov::XHelperInterface > thisHelperIface() = 0;
     /// @throws cpo::uno::RuntimeException
     void setAddIndent( const cpo::uno::Any& BAddIndent) { BAddIndent >>= mbAddIndent; }
     /// @throws cpo::uno::RuntimeException
     cpo::uno::Any getAddIndent() { return cpo::uno::Any( mbAddIndent ); }
        // Interface Methods
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual cpo::uno::Any Borders( const cpo::uno::Any& Index );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual cpo::uno::Reference< ::ooo::vba::excel::XFont > Font(  );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual cpo::uno::Reference< ::ooo::vba::excel::XInterior > Interior(  );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual void setNumberFormat( const cpo::uno::Any& NumberFormat );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual cpo::uno::Any getNumberFormat(  );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual void setNumberFormatLocal( const cpo::uno::Any& NumberFormatLocal );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual cpo::uno::Any getNumberFormatLocal(  );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual void setIndentLevel( const cpo::uno::Any& IndentLevel );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual cpo::uno::Any getIndentLevel(  );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual void setHorizontalAlignment( const cpo::uno::Any& HorizontalAlignment );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual cpo::uno::Any getHorizontalAlignment(  );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual void setVerticalAlignment( const cpo::uno::Any& VerticalAlignment );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual cpo::uno::Any getVerticalAlignment(  );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual void setOrientation( const cpo::uno::Any& Orientation );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual cpo::uno::Any getOrientation(  );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual void setShrinkToFit( const cpo::uno::Any& ShrinkToFit );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual cpo::uno::Any getShrinkToFit(  );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual void setWrapText( const cpo::uno::Any& WrapText );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual cpo::uno::Any getWrapText(  );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual void setLocked( const cpo::uno::Any& Locked );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual cpo::uno::Any getLocked(  );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual void setFormulaHidden( const cpo::uno::Any& FormulaHidden );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual cpo::uno::Any getFormulaHidden(  );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual void setMergeCells( const cpo::uno::Any& MergeCells ) = 0;
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual cpo::uno::Any getMergeCells(  ) = 0;
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual void setReadingOrder( const cpo::uno::Any& ReadingOrder );
    /// @throws css::script::BasicErrorException
    /// @throws cpo::uno::RuntimeException
    virtual cpo::uno::Any getReadingOrder(  );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
