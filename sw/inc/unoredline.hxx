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
#ifndef INCLUDED_SW_INC_UNOREDLINE_HXX
#define INCLUDED_SW_INC_UNOREDLINE_HXX

#include <unotext.hxx>

class SwRangeRedline;

/**
 * SwXRedlineText provides an XText which may be used to write
 * directly into a redline node. It got implemented to enable XML
 * import of redlines and should not be used directly via the API.
 */
class SwXRedlineText :
    public SwXText,
    public cppu::OWeakObject,
    public ::com::sun::star::container::XEnumerationAccess
{
    SwNodeIndex aNodeIndex;
    virtual const SwStartNode *GetStartNode() const;

public:
    SwXRedlineText(SwDoc* pDoc, SwNodeIndex aNodeIndex);

    virtual     ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL acquire(  ) throw(){OWeakObject::acquire();}
    virtual void SAL_CALL release(  ) throw(){OWeakObject::release();}

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    //XText
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  SAL_CALL createTextCursor(void) throw( ::com::sun::star::uno::RuntimeException, std::exception );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  SAL_CALL createTextCursorByRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & aTextPosition) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    //XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createEnumeration(void) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    //XElementAccess (via XEnumerationAccess)
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
};

typedef
cppu::WeakImplHelper1
<
    ::com::sun::star::container::XEnumerationAccess
>
SwXRedlineBaseClass;
class SwXRedline :
        public SwXRedlineBaseClass,
        public SwXText,
        public SwClient
{
    SwDoc*      pDoc;
    SwRangeRedline*  pRedline;
public:
    SwXRedline(SwRangeRedline& rRedline, SwDoc& rDoc);
    ~SwXRedline();

    TYPEINFO_OVERRIDE();

    virtual     ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL acquire(  ) throw(){OWeakObject::acquire();}
    virtual void SAL_CALL release(  ) throw(){OWeakObject::release();}

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    //XText
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  SAL_CALL createTextCursor(void) throw( ::com::sun::star::uno::RuntimeException, std::exception );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  SAL_CALL createTextCursorByRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & aTextPosition) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    //XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createEnumeration(void) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    const SwRangeRedline*    GetRedline() const {return pRedline;}
protected:
    //SwClient
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
