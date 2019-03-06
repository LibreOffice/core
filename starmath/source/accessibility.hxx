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
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Reference.h>
#include <cppuhelper/implbase.hxx>
#include <svl/SfxBroadcaster.hxx>

#include <editeng/editeng.hxx>
#include <editeng/unoedsrc.hxx>
#include <edit.hxx>
#include <view.hxx>
#include <memory>

class SmDocShell;

namespace accessibility { class AccessibleTextHelper; }

// classes and helper-classes used for accessibility in the graphic-window


typedef
cppu::WeakImplHelper
    <
        css::lang::XServiceInfo,
        css::accessibility::XAccessible,
        css::accessibility::XAccessibleComponent,
        css::accessibility::XAccessibleContext,
        css::accessibility::XAccessibleText,
        css::accessibility::XAccessibleEventBroadcaster
    >
SmGraphicAccessibleBaseClass;

class SmGraphicAccessible final :
    public SmGraphicAccessibleBaseClass
{
    OUString const                      aAccName;
    /// client id in the AccessibleEventNotifier queue
    sal_uInt32                          nClientId;

    VclPtr<SmGraphicWindow>             pWin;

    SmGraphicAccessible( const SmGraphicAccessible & ) = delete;
    SmGraphicAccessible & operator = ( const SmGraphicAccessible & ) = delete;

    SmDocShell *    GetDoc_Impl();
    OUString        GetAccessibleText_Impl();

public:
    explicit SmGraphicAccessible( SmGraphicWindow *pGraphicWin );
    virtual ~SmGraphicAccessible() override;

    void                ClearWin();     // to be called when view is destroyed
    void                LaunchEvent(
                            const sal_Int16 nAccessibleEventId,
                            const css::uno::Any &rOldVal,
                            const css::uno::Any &rNewVal);

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual css::awt::Rectangle SAL_CALL getBounds(  ) override;
    virtual css::awt::Point SAL_CALL getLocation(  ) override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) override;
    virtual css::awt::Size SAL_CALL getSize(  ) override;
    virtual void SAL_CALL grabFocus(  ) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) override;
    virtual OUString SAL_CALL getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) override;
    virtual css::lang::Locale SAL_CALL getLocale(  ) override;

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;
    virtual void SAL_CALL removeAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;

    // XAccessibleText
    virtual sal_Int32 SAL_CALL getCaretPosition(  ) override;
    virtual sal_Bool SAL_CALL setCaretPosition ( sal_Int32 nIndex ) override;
    virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes ) override;
    virtual css::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) override;
    virtual sal_Int32 SAL_CALL getCharacterCount(  ) override;
    virtual sal_Int32 SAL_CALL getIndexAtPoint( const css::awt::Point& aPoint ) override;
    virtual OUString SAL_CALL getSelectedText(  ) override;
    virtual sal_Int32 SAL_CALL getSelectionStart(  ) override;
    virtual sal_Int32 SAL_CALL getSelectionEnd(  ) override;
    virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual OUString SAL_CALL getText(  ) override;
    virtual OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
};


// classes and helper-classes used for accessibility in the command-window


class SmEditAccessible;
class SmEditSource;
class EditView;
class SvxFieldItem;


class SmViewForwarder :
    public SvxViewForwarder
{
    SmEditAccessible &          rEditAcc;

    SmViewForwarder( const SmViewForwarder & ) = delete;
    SmViewForwarder & operator = ( const SmViewForwarder & ) = delete;

public:
    explicit SmViewForwarder( SmEditAccessible &rAcc );
    virtual             ~SmViewForwarder() override;

    virtual bool        IsValid() const override;
    virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const override;
    virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const override;
};


class SmTextForwarder :     /* analog to SvxEditEngineForwarder */
    public SvxTextForwarder
{
    SmEditAccessible &  rEditAcc;
    SmEditSource &      rEditSource;

    DECL_LINK( NotifyHdl, EENotify&, void );

    SmTextForwarder( const SmTextForwarder & ) = delete;
    SmTextForwarder & operator = ( const SmTextForwarder & ) = delete;

public:
    SmTextForwarder( SmEditAccessible& rAcc, SmEditSource & rSource );
    virtual ~SmTextForwarder() override;

    virtual sal_Int32   GetParagraphCount() const override;
    virtual sal_Int32   GetTextLen( sal_Int32 nParagraph ) const override;
    virtual OUString    GetText( const ESelection& rSel ) const override;
    virtual SfxItemSet  GetAttribs( const ESelection& rSel, EditEngineAttribs nOnlyHardAttrib = EditEngineAttribs::All ) const override;
    virtual SfxItemSet  GetParaAttribs( sal_Int32 nPara ) const override;
    virtual void        SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet ) override;
    virtual void        RemoveAttribs( const ESelection& rSelection ) override;
    virtual void        GetPortions( sal_Int32 nPara, std::vector<sal_Int32>& rList ) const override;

    virtual SfxItemState  GetItemState( const ESelection& rSel, sal_uInt16 nWhich ) const override;
    virtual SfxItemState  GetItemState( sal_Int32 nPara, sal_uInt16 nWhich ) const override;

    virtual void        QuickInsertText( const OUString& rText, const ESelection& rSel ) override;
    virtual void        QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel ) override;
    virtual void        QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel ) override;
    virtual void        QuickInsertLineBreak( const ESelection& rSel ) override;

    virtual SfxItemPool* GetPool() const override;

    virtual OUString        CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, boost::optional<Color>& rpTxtColor, boost::optional<Color>& rpFldColor ) override;
    virtual void            FieldClicked(const SvxFieldItem&, sal_Int32, sal_Int32) override;
    virtual bool            IsValid() const override;

    virtual LanguageType    GetLanguage( sal_Int32, sal_Int32 ) const override;
    virtual sal_Int32       GetFieldCount( sal_Int32 nPara ) const override;
    virtual EFieldInfo      GetFieldInfo( sal_Int32 nPara, sal_uInt16 nField ) const override;
    virtual EBulletInfo     GetBulletInfo( sal_Int32 nPara ) const override;
    virtual tools::Rectangle       GetCharBounds( sal_Int32 nPara, sal_Int32 nIndex ) const override;
    virtual tools::Rectangle       GetParaBounds( sal_Int32 nPara ) const override;
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
    virtual             ~SmEditViewForwarder() override;

    virtual bool        IsValid() const override;

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
            SmEditSource( SmEditAccessible &rAcc );
    virtual ~SmEditSource() override;

    virtual std::unique_ptr<SvxEditSource> Clone() const override;
    virtual SvxTextForwarder*   GetTextForwarder() override;
    virtual SvxViewForwarder*  GetViewForwarder() override;
    virtual SvxEditViewForwarder*  GetEditViewForwarder( bool bCreate = false ) override;
    virtual void                UpdateData() override;
    virtual SfxBroadcaster&     GetBroadcaster() const override;
};


typedef
cppu::WeakImplHelper
    <
        css::lang::XServiceInfo,
        css::accessibility::XAccessible,
        css::accessibility::XAccessibleComponent,
        css::accessibility::XAccessibleContext,
        css::accessibility::XAccessibleEventBroadcaster
    >
SmEditAccessibleBaseClass;

class SmEditAccessible :
    public SmEditAccessibleBaseClass
{
    OUString const                          aAccName;
    std::unique_ptr<::accessibility::AccessibleTextHelper> pTextHelper;
    VclPtr<SmEditWindow>                    pWin;

    SmEditAccessible( const SmEditAccessible & ) = delete;
    SmEditAccessible & operator = ( const SmEditAccessible & ) = delete;

public:
    explicit SmEditAccessible( SmEditWindow *pEditWin );
    virtual ~SmEditAccessible() override;

    ::accessibility::AccessibleTextHelper *   GetTextHelper();

    void                Init();
    void                ClearWin();     // to be called when view is destroyed

    //! access EditEngine and EditView via the functions in the respective window
    //! pointers may be 0 (e.g. during reload)
    EditEngine * GetEditEngine()    { return pWin ? pWin->GetEditEngine() : nullptr; }
    EditView   * GetEditView()      { return pWin ? pWin->GetEditView() : nullptr; }

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual css::awt::Rectangle SAL_CALL getBounds(  ) override;
    virtual css::awt::Point SAL_CALL getLocation(  ) override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) override;
    virtual css::awt::Size SAL_CALL getSize(  ) override;
    virtual void SAL_CALL grabFocus(  ) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) override;
    virtual OUString SAL_CALL getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) override;
    virtual css::lang::Locale SAL_CALL getLocale(  ) override;

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;
    virtual void SAL_CALL removeAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
