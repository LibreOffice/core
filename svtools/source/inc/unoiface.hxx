/*************************************************************************
 *
 *  $RCSfile: unoiface.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sb $ $Date: 2002-07-23 12:51:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVT_UNOIFACE_HXX
#define _SVT_UNOIFACE_HXX

#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/awt/vclxwindows.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>

#include <cppuhelper/typeprovider.hxx>

#include <com/sun/star/awt/XTextArea.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/awt/XTextLayoutConstrains.hpp>

#include <svmedit.hxx>

#include <fmtfield.hxx>

namespace com { namespace sun { namespace star { namespace util {
    class XNumberFormatsSupplier;
} } } }

class SvNumberFormatsSupplierObj;

//  ----------------------------------------------------
//  class VCLXMultiLineEdit
//  ----------------------------------------------------
class VCLXMultiLineEdit :   public ::com::sun::star::awt::XTextComponent,
                            public ::com::sun::star::awt::XTextArea,
                            public ::com::sun::star::awt::XTextLayoutConstrains,
                            public VCLXWindow
{
private:
    TextListenerMultiplexer maTextListeners;

protected:
    DECL_LINK(          ModifyHdl, MultiLineEdit* );

public:
                    VCLXMultiLineEdit();
                    ~VCLXMultiLineEdit();

    void    SetWindow( Window* pWindow );

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw()  { VCLXWindow::acquire(); }
    void                                        SAL_CALL release() throw()  { VCLXWindow::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTextComponent
    void SAL_CALL addTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setText( const ::rtl::OUString& aText ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL insertText( const ::com::sun::star::awt::Selection& Sel, const ::rtl::OUString& Text ) throw(::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getText(  ) throw(::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getSelectedText(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Selection SAL_CALL getSelection(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isEditable(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setEditable( sal_Bool bEditable ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMaxTextLen( sal_Int16 nLen ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getMaxTextLen(  ) throw(::com::sun::star::uno::RuntimeException);

    //XTextArea
    ::rtl::OUString SAL_CALL getTextLines(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTextLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XVclWindowPeer
    void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);
};


/*
//  ----------------------------------------------------
//  class VCLXFileDialog
//  ----------------------------------------------------
class VCLXFileDialog : public ::com::sun::star::awt::XFileDialog, public VCLXDialog
{
public:
                    VCLXFileDialog();
                    ~VCLXFileDialog();

    // Interface:
// automatisch auskommentiert - [getIdlClass or queryInterface] - Bitte XTypeProvider benutzen!
//  sal_Bool            queryInterface( ::com::sun::star::uno::Uik aUik, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & rxInterface );

// automatisch auskommentiert - [getIdlClass or queryInterface] - Bitte XTypeProvider benutzen!
//  ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XIdlClass >     getIdlClass();


    // ::com::sun::star::awt::XFileDialog
    void            setPath( const ::rtl::OUString& rPath );
    ::rtl::OUString         getPath();

    void            setFilters( const ::com::sun::star::uno::Sequence<::rtl::OUString>& rFilterNames, const ::com::sun::star::uno::Sequence<::rtl::OUString>& rMasks );
    void            setCurrentFilter( const ::rtl::OUString& rFilterName );
    ::rtl::OUString         getCurrentFilter();

    SMART_UNO_DECLARATION( VCLXFileDialog, VCLXDialog );

};
*/

//  ----------------------------------------------------
//  class VCLXFileControl
//  ----------------------------------------------------
class VCLXFileControl : ::com::sun::star::awt::XTextComponent, public ::com::sun::star::awt::XTextLayoutConstrains, public VCLXWindow
{
protected:
    DECL_LINK(      ModifyHdl, Edit* );
    TextListenerMultiplexer maTextListeners;

public:
                    VCLXFileControl();
                    ~VCLXFileControl();

    void            SetWindow( Window* pWindow );

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw()  { VCLXWindow::acquire(); }
    void                                        SAL_CALL release() throw()  { VCLXWindow::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTextComponent
    void SAL_CALL addTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setText( const ::rtl::OUString& aText ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL insertText( const ::com::sun::star::awt::Selection& Sel, const ::rtl::OUString& Text ) throw(::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getText(  ) throw(::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getSelectedText(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Selection SAL_CALL getSelection(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isEditable(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setEditable( sal_Bool bEditable ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMaxTextLen( sal_Int16 nLen ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getMaxTextLen(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTextLayoutConstrains
    ::com::sun::star::awt::Size SAL_CALL getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class SVTXFormattedField
//  ----------------------------------------------------

class SVTXFormattedField : public VCLXSpinField
{
protected:
    SvNumberFormatsSupplierObj* m_pCurrentSupplier;
    sal_Bool                    bIsStandardSupplier;

    sal_Int32                   nKeyToSetDelayed;

    FormattedField*             GetFormattedField() const { return (FormattedField*)GetWindow(); }

public:
    SVTXFormattedField();
    ~SVTXFormattedField();

    // ::com::sun::star::awt::XVclWindowPeer
    void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >  getFormatsSupplier(void) const;
    void    setFormatsSupplier(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier > & xSupplier);
    sal_Int32   getFormatKey(void) const;
    void    setFormatKey(sal_Int32 nKey);

    void    SetValue(const ::com::sun::star::uno::Any& rValue);
    ::com::sun::star::uno::Any  GetValue();

    void    SetTreatAsNumber(sal_Bool bSet);
    sal_Bool    GetTreatAsNumber();

    void    SetDefaultValue(const ::com::sun::star::uno::Any& rValue);
    ::com::sun::star::uno::Any  GetDefaultValue();

    void    SetMinValue(const ::com::sun::star::uno::Any& rValue);
    ::com::sun::star::uno::Any  GetMinValue();

    void    SetMaxValue(const ::com::sun::star::uno::Any& rValue);
    ::com::sun::star::uno::Any  GetMaxValue();

    void    NotifyTextListeners();
    ::com::sun::star::uno::Any  convertEffectiveValue(const ::com::sun::star::uno::Any& rValue);

    virtual void    SetWindow(Window* _pWindow);
};

//  ----------------------------------------------------
//  class SVTXNumericField
//  ----------------------------------------------------
class SVTXNumericField : public ::com::sun::star::awt::XNumericField, public SVTXFormattedField
{
public:
                    SVTXNumericField();
                    ~SVTXNumericField();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw()  { SVTXFormattedField::acquire(); }
    void                                        SAL_CALL release() throw()  { SVTXFormattedField::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XNumericField
    void SAL_CALL setValue( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getValue(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMin( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getMin(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMax( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getMax(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setFirst( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getFirst(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setLast( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getLast(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setSpinSize( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getSpinSize(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setDecimalDigits( sal_Int16 nDigits ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getDecimalDigits(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isStrictFormat(  ) throw(::com::sun::star::uno::RuntimeException);
};

// ----------------------------------------------------
//  class VCLXCurrencyField
//  ----------------------------------------------------
class SVTXCurrencyField : public ::com::sun::star::awt::XCurrencyField, public SVTXFormattedField
{
public:
                    SVTXCurrencyField();
                    ~SVTXCurrencyField();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw()  { SVTXFormattedField::acquire(); }
    void                                        SAL_CALL release() throw()  { SVTXFormattedField::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XVclWindowPeer
    void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XCurrencyField
    void SAL_CALL setValue( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getValue(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMin( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getMin(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMax( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getMax(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setFirst( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getFirst(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setLast( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getLast(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setSpinSize( double Value ) throw(::com::sun::star::uno::RuntimeException);
    double SAL_CALL getSpinSize(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setDecimalDigits( sal_Int16 nDigits ) throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL getDecimalDigits(  ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL isStrictFormat(  ) throw(::com::sun::star::uno::RuntimeException);
};

//  ----------------------------------------------------
//  class VCLXProgressBar
//  ----------------------------------------------------
class VCLXProgressBar : public ::com::sun::star::awt::XProgressBar,
                        public VCLXWindow
{
private:
    sal_Int32   m_nValue;
    sal_Int32   m_nValueMin;
    sal_Int32   m_nValueMax;

protected:
    void            ImplUpdateValue();

public:
                    VCLXProgressBar();
                    ~VCLXProgressBar();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                                        SAL_CALL acquire() throw()  { VCLXWindow::acquire(); }
    void                                        SAL_CALL release() throw()  { VCLXWindow::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XProgressBar
    void SAL_CALL setForegroundColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setBackgroundColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setValue( sal_Int32 nValue ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setRange( sal_Int32 nMin, sal_Int32 nMax ) throw(::com::sun::star::uno::RuntimeException );
    sal_Int32 SAL_CALL getValue() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::VclWindowPeer
    void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException);
};

#endif  // _SVT_UNOIFACE_HXX

