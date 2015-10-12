/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

#ifndef INCLUDED_STARMATH_SOURCE_ACCESSIBILITY_HXX
#define INCLUDED_STARMATH_SOURCE_ACCESSIBILITY_HXX

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Reference.h>
#include <osl/mutex.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/implbase.hxx>
#include <svl/SfxBroadcaster.hxx>

#include <editeng/editeng.hxx>
#include <editeng/unoedsrc.hxx>
#include <svx/AccessibleTextHelper.hxx>
#include <edit.hxx>

class SmGraphicWindow;
class SmEditWindow;
class SmDocShell;

namespace com { namespace sun { namespace star { namespace accessibility {
struct AccessibleEventObject;
}}}}



// classes and helper-classes used for accessibility in the graphic-window


typedef
cppu::WeakImplHelper
    <
        com::sun::star::lang::XServiceInfo,
        com::sun::star::accessibility::XAccessible,
        com::sun::star::accessibility::XAccessibleComponent,
        com::sun::star::accessibility::XAccessibleContext,
        com::sun::star::accessibility::XAccessibleText,
        com::sun::star::accessibility::XAccessibleEventBroadcaster
    >
SmGraphicAccessibleBaseClass;

class SmGraphicAccessible :
    public SmGraphicAccessibleBaseClass
{
    OUString                            aAccName;
    /// client id in the AccessibleEventNotifier queue
    sal_uInt32                          nClientId;

    VclPtr<SmGraphicWindow>             pWin;

    SmGraphicAccessible( const SmGraphicAccessible & ) = delete;
    SmGraphicAccessible & operator = ( const SmGraphicAccessible & ) = delete;

protected:
    SmDocShell *    GetDoc_Impl();
    OUString        GetAccessibleText_Impl();

public:
    explicit SmGraphicAccessible( SmGraphicWindow *pGraphicWin );
    virtual ~SmGraphicAccessible();

    void                ClearWin();     // to be called when view is destroyed
    void                LaunchEvent(
                            const sal_Int16 nAccesibleEventId,
                            const ::com::sun::star::uno::Any &rOldVal,
                            const ::com::sun::star::uno::Any &rNewVal);

    // XAccessible
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addAccessibleEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeAccessibleEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XAccessibleText
    virtual sal_Int32 SAL_CALL getCaretPosition(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL setCaretPosition ( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< OUString >& aRequestedAttributes ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getCharacterCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getIndexAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getSelectedText(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getSelectionStart(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getSelectionEnd(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getText(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
};



// classes and helper-classes used for accessibility in the command-window


class SmEditAccessible;
class SmEditSource;
class EditEngine;
class EditView;
class SvxFieldItem;
struct ESelection;


class SmViewForwarder :
    public SvxViewForwarder
{
    SmEditAccessible &          rEditAcc;

    SmViewForwarder( const SmViewForwarder & ) = delete;
    SmViewForwarder & operator = ( const SmViewForwarder & ) = delete;

public:
    explicit SmViewForwarder( SmEditAccessible &rAcc );
    virtual             ~SmViewForwarder();

    virtual bool        IsValid() const override;
    virtual Rectangle   GetVisArea() const override;
    virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const override;
    virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const override;
};


class SmTextForwarder :     /* analog to SvxEditEngineForwarder */
    public SvxTextForwarder
{
    SmEditAccessible &  rEditAcc;
    SmEditSource &      rEditSource;

    DECL_LINK_TYPED( NotifyHdl, EENotify&, void );

    SmTextForwarder( const SmTextForwarder & ) = delete;
    SmTextForwarder & operator = ( const SmTextForwarder & ) = delete;

public:
    SmTextForwarder( SmEditAccessible& rAcc, SmEditSource & rSource );
    virtual ~SmTextForwarder();

    virtual sal_Int32   GetParagraphCount() const override;
    virtual sal_Int32   GetTextLen( sal_Int32 nParagraph ) const override;
    virtual OUString    GetText( const ESelection& rSel ) const override;
    virtual SfxItemSet  GetAttribs( const ESelection& rSel, EditEngineAttribs nOnlyHardAttrib = EditEngineAttribs_All ) const override;
    virtual SfxItemSet  GetParaAttribs( sal_Int32 nPara ) const override;
    virtual void        SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet ) override;
    virtual void        RemoveAttribs( const ESelection& rSelection, bool bRemoveParaAttribs, sal_uInt16 nWhich ) override;
    virtual void        GetPortions( sal_Int32 nPara, std::vector<sal_Int32>& rList ) const override;

    virtual SfxItemState  GetItemState( const ESelection& rSel, sal_uInt16 nWhich ) const override;
    virtual SfxItemState  GetItemState( sal_Int32 nPara, sal_uInt16 nWhich ) const override;

    virtual void        QuickInsertText( const OUString& rText, const ESelection& rSel ) override;
    virtual void        QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel ) override;
    virtual void        QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel ) override;
    virtual void        QuickInsertLineBreak( const ESelection& rSel ) override;

    virtual SfxItemPool* GetPool() const override;

    virtual OUString        CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, Color*& rpTxtColor, Color*& rpFldColor ) override;
    virtual void            FieldClicked(const SvxFieldItem&, sal_Int32, sal_Int32) override;
    virtual bool            IsValid() const override;

    virtual LanguageType    GetLanguage( sal_Int32, sal_Int32 ) const override;
    virtual sal_Int32       GetFieldCount( sal_Int32 nPara ) const override;
    virtual EFieldInfo      GetFieldInfo( sal_Int32 nPara, sal_uInt16 nField ) const override;
    virtual EBulletInfo     GetBulletInfo( sal_Int32 nPara ) const override;
    virtual Rectangle       GetCharBounds( sal_Int32 nPara, sal_Int32 nIndex ) const override;
    virtual Rectangle       GetParaBounds( sal_Int32 nPara ) const override;
    virtual MapMode         GetMapMode() const override;
    virtual OutputDevice*   GetRefDevice() const override;
    virtual bool            GetIndexAtPoint( const Point&, sal_Int32& nPara, sal_Int32& nIndex ) const override;
    virtual bool            GetWordIndices( sal_Int32 nPara, sal_Int32 nIndex, sal_Int32& nStart, sal_Int32& nEnd ) const override;
    virtual bool            GetAttributeRun( sal_Int32& nStartIndex, sal_Int32& nEndIndex, sal_Int32 nPara, sal_Int32 nIndex, bool bInCell = false ) const override;
    virtual sal_Int32       GetLineCount( sal_Int32 nPara ) const override;
    virtual sal_Int32       GetLineLen( sal_Int32 nPara, sal_Int32 nLine ) const override;
    virtual void            GetLineBoundaries( /*out*/sal_Int32 &rStart, /*out*/sal_Int32 &rEnd, sal_Int32 nParagraph, sal_Int32 nLine ) const override;
    virtual sal_Int32       GetLineNumberAtIndex( sal_Int32 nPara, sal_Int32 nLine ) const override;
    virtual bool            Delete( const ESelection& ) override;
    virtual bool            InsertText( const OUString&, const ESelection& ) override;
    virtual bool            QuickFormatDoc( bool bFull = false ) override;

    virtual sal_Int16       GetDepth( sal_Int32 nPara ) const override;
    virtual bool            SetDepth( sal_Int32 nPara, sal_Int16 nNewDepth ) override;

    virtual const SfxItemSet*   GetEmptyItemSetPtr() override;
    // implementation functions for XParagraphAppend and XTextPortionAppend
    virtual void        AppendParagraph() override;
    virtual sal_Int32   AppendTextPortion( sal_Int32 nPara, const OUString &rText, const SfxItemSet &rSet ) override;

    virtual void        CopyText(const SvxTextForwarder& rSource) override;
};


class SmEditViewForwarder :     /* analog to SvxEditEngineViewForwarder */
    public SvxEditViewForwarder
{
    SmEditAccessible&       rEditAcc;

    SmEditViewForwarder( const SmEditViewForwarder & ) = delete;
    SmEditViewForwarder & operator = ( const SmEditViewForwarder & ) = delete;

public:
    explicit SmEditViewForwarder( SmEditAccessible& rAcc );
    virtual             ~SmEditViewForwarder();

    virtual bool        IsValid() const override;

    virtual Rectangle   GetVisArea() const override;
    virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const override;
    virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const override;

    virtual bool        GetSelection( ESelection& rSelection ) const override;
    virtual bool        SetSelection( const ESelection& rSelection ) override;
    virtual bool        Copy() override;
    virtual bool        Cut() override;
    virtual bool        Paste() override;
};


class SmEditSource :
    public SvxEditSource
{
    SfxBroadcaster          aBroadCaster;
    SmViewForwarder         aViewFwd;
    SmTextForwarder         aTextFwd;
    SmEditViewForwarder     aEditViewFwd;

    SmEditAccessible&       rEditAcc;

    SmEditSource( const SmEditSource &rSrc );
    SmEditSource & operator = ( const SmEditSource & ) = delete;

public:
            SmEditSource( SmEditWindow *pWin, SmEditAccessible &rAcc );
    virtual ~SmEditSource();

    virtual SvxEditSource*      Clone() const override;
    virtual SvxTextForwarder*   GetTextForwarder() override;
    virtual SvxViewForwarder*  GetViewForwarder() override;
    virtual SvxEditViewForwarder*  GetEditViewForwarder( bool bCreate = false ) override;
    virtual void                UpdateData() override;
    virtual SfxBroadcaster&     GetBroadcaster() const override;
};




typedef
cppu::WeakImplHelper
    <
        com::sun::star::lang::XServiceInfo,
        com::sun::star::accessibility::XAccessible,
        com::sun::star::accessibility::XAccessibleComponent,
        com::sun::star::accessibility::XAccessibleContext,
        com::sun::star::accessibility::XAccessibleEventBroadcaster
    >
SmEditAccessibleBaseClass;

class SmEditAccessible :
    public SmEditAccessibleBaseClass
{
    OUString                                aAccName;
    ::accessibility::AccessibleTextHelper    *pTextHelper;
    VclPtr<SmEditWindow>                    pWin;

    SmEditAccessible( const SmEditAccessible & ) = delete;
    SmEditAccessible & operator = ( const SmEditAccessible & ) = delete;

public:
    explicit SmEditAccessible( SmEditWindow *pEditWin );
    virtual ~SmEditAccessible();

    ::accessibility::AccessibleTextHelper *   GetTextHelper() { return pTextHelper; }

    void                Init();
    void                ClearWin();     // to be called when view is destroyed

    //! access EditEngine and EditView via the functions in the respective window
    //! pointers may be 0 (e.g. during reload)
    EditEngine * GetEditEngine()    { return pWin ? pWin->GetEditEngine() : 0; }
    EditView   * GetEditView()      { return pWin ? pWin->GetEditView() : 0; }

    // XAccessible
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addAccessibleEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeAccessibleEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
