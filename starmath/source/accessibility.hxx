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

#ifdef _MSC_VER
#pragma hdrstop
#endif

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
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/implbase6.hxx>
#include <svl/brdcst.hxx>

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
cppu::WeakImplHelper6
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
    osl::Mutex                          aListenerMutex;
    OUString                            aAccName;
    /// client id in the AccessibleEventNotifier queue
    sal_uInt32                          nClientId;

    SmGraphicWindow     *pWin;

    // disallow copy-ctor and assignment-operator for now
    SmGraphicAccessible( const SmGraphicAccessible & );
    SmGraphicAccessible & operator = ( const SmGraphicAccessible & );

protected:
    SmDocShell *    GetDoc_Impl();
    OUString        GetAccessibleText_Impl();

public:
    SmGraphicAccessible( SmGraphicWindow *pGraphicWin );
    virtual ~SmGraphicAccessible();

    SmGraphicWindow *   GetWin()    { return pWin; }
    void                ClearWin();     // to be called when view is destroyed
    void                LaunchEvent(
                            const sal_Int16 nAccesibleEventId,
                            const ::com::sun::star::uno::Any &rOldVal,
                            const ::com::sun::star::uno::Any &rNewVal);

    // XAccessible
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addAccessibleEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removeAccessibleEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    // XAccessibleText
    virtual sal_Int32 SAL_CALL getCaretPosition(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL setCaretPosition ( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< OUString >& aRequestedAttributes ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getCharacterBounds( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Int32 SAL_CALL getCharacterCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Int32 SAL_CALL getIndexAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual OUString SAL_CALL getSelectedText(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Int32 SAL_CALL getSelectionStart(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Int32 SAL_CALL getSelectionEnd(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual OUString SAL_CALL getText(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual OUString SAL_CALL getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
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

    // disallow copy-ctor and assignment-operator for now
    SmViewForwarder( const SmViewForwarder & );
    SmViewForwarder & operator = ( const SmViewForwarder & );

public:
                        SmViewForwarder( SmEditAccessible &rAcc );
    virtual             ~SmViewForwarder();

    virtual bool        IsValid() const;
    virtual Rectangle   GetVisArea() const;
    virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const;
    virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const;
};


class SmTextForwarder :     /* analog to SvxEditEngineForwarder */
    public SvxTextForwarder
{
    SmEditAccessible &  rEditAcc;
    SmEditSource &      rEditSource;

    DECL_LINK( NotifyHdl, EENotify * );

    // disallow copy-ctor and assignment-operator for now
    SmTextForwarder( const SmTextForwarder & );
    SmTextForwarder & operator = ( const SmTextForwarder & );

public:
    SmTextForwarder( SmEditAccessible& rAcc, SmEditSource & rSource );
    virtual ~SmTextForwarder();

    virtual sal_Int32   GetParagraphCount() const;
    virtual sal_Int32   GetTextLen( sal_Int32 nParagraph ) const;
    virtual OUString    GetText( const ESelection& rSel ) const;
    virtual SfxItemSet  GetAttribs( const ESelection& rSel, sal_Bool bOnlyHardAttrib = EditEngineAttribs_All ) const;
    virtual SfxItemSet  GetParaAttribs( sal_Int32 nPara ) const;
    virtual void        SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet );
    virtual void        RemoveAttribs( const ESelection& rSelection, bool bRemoveParaAttribs, sal_uInt16 nWhich );
    virtual void        GetPortions( sal_Int32 nPara, std::vector<sal_Int32>& rList ) const;

    virtual sal_uInt16      GetItemState( const ESelection& rSel, sal_uInt16 nWhich ) const;
    virtual sal_uInt16      GetItemState( sal_Int32 nPara, sal_uInt16 nWhich ) const;

    virtual void        QuickInsertText( const OUString& rText, const ESelection& rSel );
    virtual void        QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel );
    virtual void        QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel );
    virtual void        QuickInsertLineBreak( const ESelection& rSel );

    virtual SfxItemPool* GetPool() const;

    virtual OUString        CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, Color*& rpTxtColor, Color*& rpFldColor );
    virtual void            FieldClicked(const SvxFieldItem&, sal_Int32, sal_Int32);
    virtual bool            IsValid() const;

    virtual LanguageType    GetLanguage( sal_Int32, sal_Int32 ) const;
    virtual sal_Int32       GetFieldCount( sal_Int32 nPara ) const;
    virtual EFieldInfo      GetFieldInfo( sal_Int32 nPara, sal_uInt16 nField ) const;
    virtual EBulletInfo     GetBulletInfo( sal_Int32 nPara ) const;
    virtual Rectangle       GetCharBounds( sal_Int32 nPara, sal_Int32 nIndex ) const;
    virtual Rectangle       GetParaBounds( sal_Int32 nPara ) const;
    virtual MapMode         GetMapMode() const;
    virtual OutputDevice*   GetRefDevice() const;
    virtual bool            GetIndexAtPoint( const Point&, sal_Int32& nPara, sal_Int32& nIndex ) const;
    virtual bool            GetWordIndices( sal_Int32 nPara, sal_Int32 nIndex, sal_Int32& nStart, sal_Int32& nEnd ) const;
    virtual bool            GetAttributeRun( sal_Int32& nStartIndex, sal_Int32& nEndIndex, sal_Int32 nPara, sal_Int32 nIndex, bool bInCell = false ) const;
    virtual sal_Int32       GetLineCount( sal_Int32 nPara ) const;
    virtual sal_Int32       GetLineLen( sal_Int32 nPara, sal_Int32 nLine ) const;
    virtual void            GetLineBoundaries( /*out*/sal_Int32 &rStart, /*out*/sal_Int32 &rEnd, sal_Int32 nParagraph, sal_Int32 nLine ) const;
    virtual sal_Int32       GetLineNumberAtIndex( sal_Int32 nPara, sal_Int32 nLine ) const;
    virtual bool            Delete( const ESelection& );
    virtual bool            InsertText( const OUString&, const ESelection& );
    virtual bool            QuickFormatDoc( bool bFull = false );

    virtual sal_Int16       GetDepth( sal_Int32 nPara ) const;
    virtual bool            SetDepth( sal_Int32 nPara, sal_Int16 nNewDepth );

    virtual const SfxItemSet*   GetEmptyItemSetPtr();
    // implementation functions for XParagraphAppend and XTextPortionAppend
    virtual void        AppendParagraph();
    virtual sal_Int32   AppendTextPortion( sal_Int32 nPara, const OUString &rText, const SfxItemSet &rSet );

    virtual void        CopyText(const SvxTextForwarder& rSource);
};


class SmEditViewForwarder :     /* analog to SvxEditEngineViewForwarder */
    public SvxEditViewForwarder
{
    SmEditAccessible&       rEditAcc;

    // disallow copy-ctor and assignment-operator for now
    SmEditViewForwarder( const SmEditViewForwarder & );
    SmEditViewForwarder & operator = ( const SmEditViewForwarder & );

public:
                        SmEditViewForwarder( SmEditAccessible& rAcc );
    virtual             ~SmEditViewForwarder();

    virtual bool        IsValid() const;

    virtual Rectangle   GetVisArea() const;
    virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const;
    virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const;

    virtual bool        GetSelection( ESelection& rSelection ) const;
    virtual bool        SetSelection( const ESelection& rSelection );
    virtual bool        Copy();
    virtual bool        Cut();
    virtual bool        Paste();
};


class SmEditSource :
    public SvxEditSource
{
    SfxBroadcaster          aBroadCaster;
    SmViewForwarder         aViewFwd;
    SmTextForwarder         aTextFwd;
    SmEditViewForwarder     aEditViewFwd;

    SmEditAccessible&       rEditAcc;

    // disallow copy-ctor and assignment-operator for now
    SmEditSource( const SmEditSource &rSrc );
    SmEditSource & operator = ( const SmEditSource & );

public:
            SmEditSource( SmEditWindow *pWin, SmEditAccessible &rAcc );
    virtual ~SmEditSource();

    virtual SvxEditSource*      Clone() const;
    virtual SvxTextForwarder*   GetTextForwarder();
    virtual SvxViewForwarder*  GetViewForwarder();
    virtual SvxEditViewForwarder*  GetEditViewForwarder( bool bCreate = false ) SAL_OVERRIDE;
    virtual void                UpdateData();
    virtual SfxBroadcaster&     GetBroadcaster() const;
};




typedef
cppu::WeakImplHelper5
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
    osl::Mutex                              aListenerMutex;
    OUString                                aAccName;
    ::accessibility::AccessibleTextHelper    *pTextHelper;
    SmEditWindow                           *pWin;

    // disallow copy-ctor and assignment-operator for now
    SmEditAccessible( const SmEditAccessible & );
    SmEditAccessible & operator = ( const SmEditAccessible & );

public:
    SmEditAccessible( SmEditWindow *pEditWin );
    virtual ~SmEditAccessible();

    ::accessibility::AccessibleTextHelper *   GetTextHelper() { return pTextHelper; }

    void                Init();
    SmEditWindow *      GetWin()    { return pWin; }
    void                ClearWin();     // to be called when view is destroyed

    //! access EditEngine and EditView via the functions in the respective window
    //! pointers may be 0 (e.g. during reload)
    EditEngine * GetEditEngine()    { return pWin ? pWin->GetEditEngine() : 0; }
    EditView   * GetEditView()      { return pWin ? pWin->GetEditView() : 0; }

    // XAccessible
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addAccessibleEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removeAccessibleEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
