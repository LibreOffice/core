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


#ifndef SC_VBA_FONT_HXX
#define SC_VBA_FONT_HXX

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/excel/XFont.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbafontbase.hxx>
#include "vbapalette.hxx"

class ScTableSheetsObj;
class ScCellRangeObj;

typedef cppu::ImplInheritanceHelper1< VbaFontBase, ov::excel::XFont > ScVbaFont_BASE;

class ScVbaFont : public ScVbaFont_BASE
{
    ScVbaPalette mPalette;
    ScCellRangeObj* mpRangeObj;
    SfxItemSet*  GetDataSet();
public:
    ScVbaFont(
        const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const ScVbaPalette& dPalette,
        const css::uno::Reference< css::beans::XPropertySet >& xPropertySet,
        ScCellRangeObj* pRangeObj = 0, bool bFormControl = false ) throw ( css::uno::RuntimeException );
    virtual ~ScVbaFont();// {}

    // Attributes
    virtual css::uno::Any SAL_CALL getSize() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getStandardFontSize() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setStandardFontSize( const css::uno::Any& _standardfontsize ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getStandardFont() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setStandardFont( const css::uno::Any& _standardfont ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getFontStyle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setFontStyle( const css::uno::Any& _fontstyle ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getColorIndex() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setColorIndex( const css::uno::Any& _colorindex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getBold() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getUnderline() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setUnderline( const css::uno::Any& _underline ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getStrikethrough() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getShadow() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getItalic() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getSubscript() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSubscript( const css::uno::Any& _subscript ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getSuperscript() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSuperscript( const css::uno::Any& _superscript ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getColor() throw (css::uno::RuntimeException) ;
    virtual css::uno::Any SAL_CALL getOutlineFont() throw (css::uno::RuntimeException) ;
    virtual void SAL_CALL setOutlineFont( const css::uno::Any& _outlinefont ) throw (css::uno::RuntimeException) ;
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();

};



#endif /* SC_VBA_FONT_HXX */

