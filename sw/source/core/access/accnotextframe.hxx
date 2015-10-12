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

#ifndef INCLUDED_SW_SOURCE_CORE_ACCESS_ACCNOTEXTFRAME_HXX
#define INCLUDED_SW_SOURCE_CORE_ACCESS_ACCNOTEXTFRAME_HXX

#include "accframebase.hxx"
#include <com/sun/star/accessibility/XAccessibleImage.hpp>
#include <com/sun/star/accessibility/XAccessibleHypertext.hpp>

namespace utl { class AccessibleRelationSetHelper; }
namespace com { namespace star {
    namespace accessibility { struct AccessibleRelation; }
} }

class SwFlyFrm;
class SwNoTextNode;
class SwAccessibleNoTextHyperlink;

class SwAccessibleNoTextFrame : public  SwAccessibleFrameBase,
                                public ::com::sun::star::accessibility::XAccessibleImage,
                                public ::com::sun::star::accessibility::XAccessibleHypertext//Added by yangzhh for HyperLink
{
    friend class SwAccessibleNoTextHyperlink;
    //HyperLinksMap alinksMap;
    com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleHyperlink > alink;
    SwDepend        aDepend;
    OUString msTitle; // #i73249#
    OUString msDesc;

protected:
    virtual ~SwAccessibleNoTextFrame();

    const SwNoTextNode *GetNoTextNode() const;

    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;

public:
    SwAccessibleNoTextFrame( SwAccessibleMap* pInitMap,
                             sal_Int16 nInitRole,
                             const SwFlyFrm *pFlyFrm );

    // XAccessibleContext

    // #i73249# - Return the object's current name.
    virtual OUString SAL_CALL
        getAccessibleName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return this object's description.
    virtual OUString SAL_CALL
        getAccessibleDescription()
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    // XInterface methods need to be implemented to disambiguate
    // between those inherited through SwAcessibleContext and
    // XAccessibleImage.

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL acquire(  ) throw () override
        { SwAccessibleContext::acquire(); };

    virtual void SAL_CALL release(  ) throw () override
        { SwAccessibleContext::release(); };

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XAccessibleImage
    virtual OUString SAL_CALL
        getAccessibleImageDescription(  )
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL
        getAccessibleImageHeight(  )
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL
        getAccessibleImageWidth(  )
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // The object is not visible an longer and should be destroyed
    virtual void Dispose( bool bRecursive = false ) override;

    virtual sal_Int32 SAL_CALL getCaretPosition(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL setCaretPosition( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Unicode SAL_CALL getCharacter( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;//Shen Zhen Jie changed sal_Unicode to sal_uInt32
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

    // XAccessibleHypertext
    virtual sal_Int32 SAL_CALL getHyperLinkCount()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleHyperlink >
    SAL_CALL getHyperLink( sal_Int32 nLinkIndex )
    throw (::com::sun::star::lang::IndexOutOfBoundsException,
            ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getHyperLinkIndex( sal_Int32 nCharIndex )
    throw (::com::sun::star::lang::IndexOutOfBoundsException,
            ::com::sun::star::uno::RuntimeException, std::exception) override;

    SwAccessibleMap *GetAccessibleMap(){ return GetMap();}

public:
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
