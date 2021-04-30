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


#ifndef INCLUDED_SVX_ACCESSIBLESHAPE_HXX
#define INCLUDED_SVX_ACCESSIBLESHAPE_HXX

#include <com/sun/star/accessibility/AccessibleScrollType.hpp>
#include <com/sun/star/accessibility/TextSegment.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleExtendedAttributes.hpp>
#include <com/sun/star/accessibility/XAccessibleGroupPosition.hpp>
#include <com/sun/star/accessibility/XAccessibleHypertext.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/document/XShapeEventListener.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <comphelper/servicehelper.hxx>
#include <editeng/AccessibleContextBase.hxx>
#include <editeng/AccessibleComponentBase.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <svx/AccessibleShapeTreeInfo.hxx>
#include <svx/IAccessibleViewForwarderListener.hxx>
#include <svx/svxdllapi.h>
#include <memory>

namespace com::sun::star {
    namespace accessibility { class XAccessible; }
    namespace accessibility { class XAccessibleEventListener; }
    namespace accessibility { class XAccessibleHyperlink; }
    namespace accessibility { class XAccessibleRelationSet; }
    namespace accessibility { class XAccessibleStateSet; }
    namespace beans { struct PropertyValue; }
    namespace document { struct EventObject; }
    namespace drawing { class XShape; }
    namespace uno { class XInterface; }
}

class SdrObject;

namespace accessibility {

class AccessibleShapeInfo;
class AccessibleTextHelper;
class ChildrenManager;
class IAccessibleParent;

/** This base class provides a base implementation for all shapes.  For more
    detailed documentation about the methods refer to the descriptions of
    the implemented interfaces.  These are, among others,
    XAccessible, <type>XAccessibleContext</type>,
    XAccessibleComponent and
    XAccessibleExtendedComponent.

    <p>The children of a shape can stem from two sources which, in case of
    SVX and SD shapes, are mutually exclusive.  This implementation,
    however, handles both simultaneously to cope with future extensions or
    shapes from other projects.
    <ul>
        <li>If this shape is a group shape, i.e. a
        SvxShapeGroup or a <type>Svx3DSceneObject</type>, it
        can have nested shapes.</li>
        <li>If this shape is a descendant from SvxShapeText
        then the text paragraphs are its children.</li>
    </ul>
    </p>

    <p>Accessible shapes do not listen for disposing() calls of the UNO
    shapes they make accessible.  This is the task of their owner, usually a
    container, who can then call dispose() at the accessible object.</p>
*/
class SVX_DLLPUBLIC AccessibleShape
    :   public AccessibleContextBase,
        public AccessibleComponentBase,
        public css::accessibility::XAccessibleSelection,
        public css::accessibility::XAccessibleExtendedAttributes,
        public css::accessibility::XAccessibleGroupPosition,
        public css::accessibility::XAccessibleHypertext,
        public IAccessibleViewForwarderListener,
        public css::document::XShapeEventListener,
        public css::lang::XUnoTunnel
{
public:
    //=====  internal  ========================================================

    /** Create a new accessible object that makes the given shape accessible.
        @param rShapeInfo
            This object contains all information specific to the new
            accessible shape.  That are e.g. the shape to be made accessible
            and the accessible object that will become the parent of the new
            object.
        @param rShapeTreeInfo
            Bundle of information passed to this shape and all of its descendants.
        @attention
            Always call the <member>init</member> method after creating a
            new accessible shape.  This is one way to overcome the potential
            problem of registering the new object with e.g. event
            broadcasters.  That would delete the new object if a broadcaster
            would not keep a strong reference to the new object.
    */
    AccessibleShape (
        const AccessibleShapeInfo& rShapeInfo,
        const AccessibleShapeTreeInfo& rShapeTreeInfo);
     //Solution: Overwrite the object's current name.
    virtual OUString SAL_CALL    getAccessibleName() override;
    virtual OUString SAL_CALL    getAccessibleDescription() override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet> SAL_CALL getAccessibleRelationSet() override;
//=====  XAccessibleSelection  ============================================

    virtual void SAL_CALL selectAccessibleChild(
        sal_Int32 nChildIndex ) override;

    virtual sal_Bool SAL_CALL isAccessibleChildSelected(
        sal_Int32 nChildIndex ) override;

    virtual void SAL_CALL clearAccessibleSelection(  ) override;

    virtual void SAL_CALL selectAllAccessibleChildren(  ) override;

    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild(
        sal_Int32 nSelectedChildIndex ) override;

    virtual void SAL_CALL deselectAccessibleChild(
        sal_Int32 nSelectedChildIndex ) override;

    // ====== XAccessibleExtendedAttributes =====================================
    virtual css::uno::Any SAL_CALL getExtendedAttributes() override ;
    /// Return this object's role.
    virtual sal_Int16 SAL_CALL getAccessibleRole() override;
    //=====  XAccessibleGroupPosition  =========================================
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL
        getGroupPosition( const css::uno::Any& rAny ) override;
    virtual OUString SAL_CALL getObjectLink( const css::uno::Any& accoject ) override;
    /** The destructor releases its children manager and text engine if
        still existent.  These are responsible to send appropriate events.
    */
    virtual ~AccessibleShape() override;

    /** Initialize a new shape.  See the documentation of the constructor
        for the reason of this method's existence.
    */
    virtual void Init();

    /** Set the specified state.  If the state is <const>FOCUSED</const>
        then, additionally to the inherited functionality, the focus
        listeners registered with the XAccessibleComponent
        interface are called (if that state really changes).

        @param aState
            The state to turn on.

        @return
            The returned flag indicates whether the specified state has been
            changed (<TRUE/>), i.e. it has formerly not been set.
    */
    virtual bool SetState (sal_Int16 aState) override;

    /** Reset the specified state.  If the state is <const>FOCUSED</const>
        then, additionally to the inherited functionality, the focus
        listeners registered with the XAccessibleComponent
        interface are called (if that state really changes).

        @param aState
            The state to turn off.

        @return
            The returned flag indicates whether the specified state has been
            changed (<TRUE/>), i.e. it has formerly been set.
    */
    virtual bool ResetState (sal_Int16 aState) override;

    /** Return the state of the specified state.  Take the
        <const>FOCUSED</const> state from the accessible edit engine.

        @param aState
            The state for which to return its value.
        @return
            A value of <TRUE/> indicates that the state is set.  A <FALSE/>
            value indicates an unset state or the inability to access the
            entity that manages the state set.

    */
    bool GetState (sal_Int16 aState);


    //=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount() override;

    /** Return the specified child.
        @param nIndex
            Index of the requested child.
        @return
            Reference of the requested child which is the accessible object
            of a visible shape.
        @throws IndexOutOfBoundsException
            Throws an exception if the index is not valid.
    */
    virtual css::uno::Reference<
            css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 nIndex) override;


    /// Return the set of current states.
    virtual css::uno::Reference<
            css::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet() override;

    /// Return this objects index among the parents children.
    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent() override;

    //=====  XAccessibleComponent  ============================================

    virtual css::uno::Reference<
        css::accessibility::XAccessible > SAL_CALL
        getAccessibleAtPoint (const css::awt::Point& aPoint) override;

    virtual css::awt::Rectangle SAL_CALL getBounds() override;

    virtual css::awt::Point SAL_CALL getLocation() override;

    virtual css::awt::Point SAL_CALL getLocationOnScreen() override;

    virtual css::awt::Size SAL_CALL getSize() override;

    virtual sal_Int32 SAL_CALL getForeground() override;

    virtual sal_Int32 SAL_CALL getBackground() override;

    //=====  XAccessibleEventBroadcaster  =====================================

    /** This call is forwarded to a) the base class and b) to the
        accessible edit engine if it is present.

        @param rxListener
            This listener is informed about accessibility events.
    */
    virtual void SAL_CALL
        addAccessibleEventListener (
            const css::uno::Reference<
            css::accessibility::XAccessibleEventListener >& rxListener) override;

    /** This call is forwarded to a) the base class and b) to the
        accessible edit engine if it is present.

        @param rxListener
            This listener will not be informed about accessibility events
            anymore.
    */
    virtual void SAL_CALL
        removeAccessibleEventListener (
            const css::uno::Reference<
            css::accessibility::XAccessibleEventListener >& rxListener) override;


    //=====  XInterface  ======================================================

    virtual css::uno::Any SAL_CALL
        queryInterface (const css::uno::Type & rType) override;

    virtual void SAL_CALL
        acquire()
        noexcept override;

    virtual void SAL_CALL
        release()
        noexcept override;


    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName() override;

    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames() override;

    //=====  XTypeProvider  ===================================================

    virtual css::uno::Sequence< css::uno::Type> SAL_CALL
        getTypes() override;

    //=====  IAccessibleViewForwarderListener  ================================
    virtual void ViewForwarderChanged() override;

    /** Listen for disposing events of the model.  The accessible shape
        remains functional when this happens.
    */
    void disposing (const css::lang::EventObject& Source);

    //=====  document::XShapeEventListener  ========================================

    virtual void SAL_CALL
        notifyShapeEvent (const css::document::EventObject& rEventObject) override;


    //===== XUnoTunnel ========================================================

    UNO3_GETIMPLEMENTATION_DECL(AccessibleShape)

    //===== XAccessibleHypertext ========================================================
    virtual sal_Int32 SAL_CALL getHyperLinkCount() override;
    virtual css::uno::Reference< css::accessibility::XAccessibleHyperlink >
        SAL_CALL getHyperLink( sal_Int32 nLinkIndex ) override;
    virtual sal_Int32 SAL_CALL getHyperLinkIndex( sal_Int32 nCharIndex ) override;
    //=====  XAccessibleText  ==================================================
        virtual sal_Int32 SAL_CALL getCaretPosition(  ) override;
        virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) override;
        virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) override;//Shen Zhen Jie changed sal_Unicode to sal_uInt32; change back to sal_Unicode
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
        virtual sal_Bool SAL_CALL scrollSubstringTo( sal_Int32 nStartIndex, sal_Int32 nEndIndex, css::accessibility::AccessibleScrollType aScrollType) override;

    //===== Misc ========================================================

    const css::uno::Reference< css::drawing::XShape >&
        GetXShape() const { return mxShape; }

    /** set the index _nIndex at the accessible shape
        @param  _nIndex
            The new index in parent.
    */
    void setIndexInParent(sal_Int32 _nIndex) { m_nIndexInParent = _nIndex; }

protected:
    /// Children manager. May be empty if there are no children.
    std::unique_ptr<ChildrenManager> mpChildrenManager;

    /// Reference to the actual shape.
    css::uno::Reference<
        css::drawing::XShape> mxShape;

    /** Bundle of information passed to all shapes in a document tree.
    */
    AccessibleShapeTreeInfo maShapeTreeInfo;

    /** the index in parent.
    */
    sal_Int32 m_nIndexInParent;

    /** The accessible text engine.  May be NULL if it can not be created.
    */
    std::unique_ptr<AccessibleTextHelper> mpText;

    /** This object can be used to modify the child list of our parent.
    */
    IAccessibleParent* mpParent;

    /** This object can be removed when we have an extra interface to ask if the shape is selected
    */
    SdrObject* m_pShape;

    /** This method is called from the component helper base class while
        disposing.
    */
    virtual void SAL_CALL disposing() override;

    /** Create a base name string that contains the accessible name.

        @throws css::uno::RuntimeException
    */
    virtual OUString
        CreateAccessibleBaseName();

    /** Create a unique name string that contains the accessible name.  The
        name consists of the base name and the index.
    */
    virtual OUString
        CreateAccessibleName() override;

    /// @throws css::uno::RuntimeException
    OUString
       GetFullAccessibleName(AccessibleShape *shape);
    virtual OUString GetStyle();
    /** Update the <const>OPAQUE</const> and <const>SELECTED</const> state.
    */
    void UpdateStates();

private:
    AccessibleShape (const AccessibleShape&) = delete;
    AccessibleShape& operator= (const AccessibleShape&) = delete;
    //Old accessible name
    OUString aAccName;

    /** Call this method when the title, name, or description of the mxShape
        member (may) have been changed.
        This method adapts the name and description members of the
        AccessibleContextBase base class.
    */
    void UpdateNameAndDescription();
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
