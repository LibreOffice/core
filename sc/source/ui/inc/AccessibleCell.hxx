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




#ifndef _SC_ACCESSIBLECELL_HXX
#define _SC_ACCESSIBLECELL_HXX

#include "AccessibleCellBase.hxx"
#include "global.hxx"
#include "viewdata.hxx"
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <unotools/accessiblerelationsethelper.hxx>
#include <editeng/AccessibleStaticTextBase.hxx>
#include <comphelper/uno3.hxx>
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XAccessibleExtendedAttributes_HPP_
#include <com/sun/star/accessibility/XAccessibleExtendedAttributes.hpp>
#endif

class ScTabViewShell;
class ScAccessibleDocument;

typedef cppu::ImplHelper1< ::com::sun::star::accessibility::XAccessibleExtendedAttributes>
                    ScAccessibleCellAttributeImpl;

/** @descr
        This base class provides an implementation of the
        <code>AccessibleCell</code> service.
*/
class ScAccessibleCell
    :   public  ScAccessibleCellBase,
        public  accessibility::AccessibleStaticTextBase,
        public  ScAccessibleCellAttributeImpl
{
public:
    //=====  internal  ========================================================
    ScAccessibleCell(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>& rxParent,
        ScTabViewShell* pViewShell,
        ScAddress& rCellAddress,
        sal_Int32 nIndex,
        ScSplitPos eSplitPos,
        ScAccessibleDocument* pAccDoc);

    virtual void Init();

    using ScAccessibleCellBase::disposing;
    virtual void SAL_CALL disposing();

protected:
    virtual ~ScAccessibleCell();

    using ScAccessibleCellBase::IsDefunc;

public:
    ///=====  XInterface  =====================================================

    DECLARE_XINTERFACE()

    ///=====  XTypeProvider  ===================================================

    DECLARE_XTYPEPROVIDER()

    ///=====  XAccessibleComponent  ============================================

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        SAL_CALL getAccessibleAtPoint(
        const ::com::sun::star::awt::Point& rPoint )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL grabFocus(  )
        throw (::com::sun::star::uno::RuntimeException);

protected:
    /// Return the object's current bounding box relative to the desktop.
    virtual Rectangle GetBoundingBoxOnScreen(void) const
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current bounding box relative to the parent object.
    virtual Rectangle GetBoundingBox(void) const
        throw (::com::sun::star::uno::RuntimeException);

public:
    ///=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    // is overloaded to calculate this on demand
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount(void)
                    throw (::com::sun::star::uno::RuntimeException);

    /// Return the specified child or NULL if index is invalid.
    // is overloaded to calculate this on demand
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild(sal_Int32 nIndex)
        throw (::com::sun::star::uno::RuntimeException,
                ::com::sun::star::lang::IndexOutOfBoundsException);

    /// Return the set of current states.
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet(void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleRelationSet> SAL_CALL
           getAccessibleRelationSet(void)
        throw (::com::sun::star::uno::RuntimeException);

    ///=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual ::rtl::OUString SAL_CALL
        getImplementationName(void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a list of all supported services.
    */
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames(void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any SAL_CALL getExtendedAttributes()
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException) ;

    // Override this method to handle cell's ParaIndent attribute specially.
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getCharacterAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRequestedAttributes )
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
private:
    ScTabViewShell* mpViewShell;
    ScAccessibleDocument* mpAccDoc;

    ScSplitPos meSplitPos;

    sal_Bool IsDefunc(
        const com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);
    virtual sal_Bool IsEditable(
        const com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);
    sal_Bool IsOpaque(
        const com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);
    sal_Bool IsSelected();

    ScDocument* GetDocument(ScTabViewShell* mpViewShell);

    ::std::auto_ptr< SvxEditSource > CreateEditSource(ScTabViewShell* pViewShell, ScAddress aCell, ScSplitPos eSplitPos);

    void FillDependends(utl::AccessibleRelationSetHelper* pRelationSet);
    void FillPrecedents(utl::AccessibleRelationSetHelper* pRelationSet);
    void AddRelation(const ScAddress& rCell,
        const sal_uInt16 aRelationType,
        ::utl::AccessibleRelationSetHelper* pRelationSet);
    void AddRelation(const ScRange& rRange,
        const sal_uInt16 aRelationType,
        ::utl::AccessibleRelationSetHelper* pRelationSet);
    sal_Bool IsFormulaMode();
    sal_Bool IsDropdown();
};


#endif
