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




#ifndef _SC_ACCESSIBLEDOCUMENTPAGEPREVIEW_HXX
#define _SC_ACCESSIBLEDOCUMENTPAGEPREVIEW_HXX

#include "AccessibleDocumentBase.hxx"

class ScPreviewShell;
class ScNotesChilds;
class ScShapeChilds;
class ScAccessiblePreviewTable;
class ScAccessiblePageHeader;

class ScAccessibleDocumentPagePreview
    :   public ScAccessibleDocumentBase
{
public:
    //=====  internal  ========================================================
    ScAccessibleDocumentPagePreview(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent,
        ScPreviewShell* pViewShell );
protected:
    virtual ~ScAccessibleDocumentPagePreview(void);

    using ScAccessibleDocumentBase::IsDefunc;

public:
    using ScAccessibleContextBase::disposing;

     virtual void SAL_CALL disposing();

   ///=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    ///=====  XAccessibleComponent  ============================================

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        SAL_CALL getAccessibleAtPoint(
        const ::com::sun::star::awt::Point& rPoint )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL grabFocus(  )
        throw (::com::sun::star::uno::RuntimeException);

    ///=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the specified child or NULL if index is invalid.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild(sal_Int32 nIndex)
        throw (::com::sun::star::uno::RuntimeException,
                ::com::sun::star::lang::IndexOutOfBoundsException);

    /// Return the set of current states.
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet(void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getAccessibleName(void)
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

    ///=====  XTypeProvider  ===================================================

    /** Returns a implementation id.
    */
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId(void)
        throw (::com::sun::star::uno::RuntimeException);

    ///=====  internal  ========================================================

//UNUSED2009-05 com::sun::star::uno::Reference < com::sun::star::accessibility::XAccessible >
//UNUSED2009-05     GetCurrentAccessibleTable();

//UNUSED2009-05 void ChildCountChanged();

protected:
    /// Return this object's description.
    virtual ::rtl::OUString SAL_CALL
        createAccessibleDescription(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current name.
    virtual ::rtl::OUString SAL_CALL
        createAccessibleName(void)
        throw (::com::sun::star::uno::RuntimeException);

public: // needed in ScShapeChilds
    /// Return the object's current bounding box relative to the desktop.
    virtual Rectangle GetBoundingBoxOnScreen(void) const
        throw (::com::sun::star::uno::RuntimeException);

protected:
    /// Return the object's current bounding box relative to the parent object.
    virtual Rectangle GetBoundingBox(void) const
        throw (::com::sun::star::uno::RuntimeException);

private:
    ScPreviewShell* mpViewShell;
    ScNotesChilds* mpNotesChilds;
    ScShapeChilds* mpShapeChilds;
    ScAccessiblePreviewTable* mpTable;
    ScAccessiblePageHeader* mpHeader;
    ScAccessiblePageHeader* mpFooter;

    sal_Bool IsDefunc(
        const com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);

    ScNotesChilds* GetNotesChilds();
    ScShapeChilds* GetShapeChilds();
};


#endif
