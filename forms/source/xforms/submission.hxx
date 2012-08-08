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

#ifndef _SUBMISSION_HXX
#define _SUBMISSION_HXX

#include <cppuhelper/implbase2.hxx>
#include <propertysetbase.hxx>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/xml/dom/events/XEventListener.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XDocumentFragment.hpp>
#include <com/sun/star/xml/xpath/XXPathObject.hpp>
#include <com/sun/star/xforms/XSubmission.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <computedexpression.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

// forward declaractions
namespace com { namespace sun { namespace star {
    namespace xforms { class XModel; }
    namespace uno { class Any; }
    namespace beans { class UnknownPropertyException;
                      class PropertyVetoException; }
    namespace lang { class IllegalArgumentException;
                     class WrappedTargetException; }
} } }
namespace xforms { class Model; }


namespace xforms
{

/** An XForms submission.
 *
 * See http://www.w3.org/TR/xforms/ for more information.
 */
typedef cppu::ImplInheritanceHelper2<
    PropertySetBase,
    com::sun::star::lang::XUnoTunnel,
    com::sun::star::xforms::XSubmission
> Submission_t;

class Submission : public Submission_t
{
    // submission properties
    rtl::OUString msID;
    rtl::OUString msBind;
    ComputedExpression maRef;
    rtl::OUString msAction;
    rtl::OUString msMethod;
    rtl::OUString msVersion;
    bool mbIndent;
    rtl::OUString msMediaType;
    rtl::OUString msEncoding;
    bool mbOmitXmlDeclaration;
    bool mbStandalone;
    rtl::OUString msCDataSectionElement;
    rtl::OUString msReplace;
    rtl::OUString msSeparator;
    com::sun::star::uno::Sequence< rtl::OUString > msIncludeNamespacePrefixes;

private:

    /// the Model to which this Submission belongs; may be NULL
    com::sun::star::uno::Reference<com::sun::star::xforms::XModel> mxModel;

    // this will extract the document from the model that will be submitted
    com::sun::star::uno::Reference< com::sun::star::xml::dom::XDocumentFragment >
        createSubmissionDocument(const com::sun::star::uno::Reference< com::sun::star::xml::xpath::XXPathObject >& aObject,
                                 sal_Bool bRemoveWSNodes = sal_False);
    com::sun::star::uno::Reference< com::sun::star::xml::dom::XDocument >
        getInstanceDocument(const com::sun::star::uno::Reference< com::sun::star::xml::xpath::XXPathObject >& aObject);

    com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory > m_aFactory;
public:
    Submission();
    virtual ~Submission() throw();

    //
    // property methods: get/set value
    //

    /// get XForms model
    com::sun::star::uno::Reference<com::sun::star::xforms::XModel>
        getModel() const;

    /// set XForms model
    void setModel(
        const com::sun::star::uno::Reference<com::sun::star::xforms::XModel>& );

    rtl::OUString getID() const;         /// get ID for this submission
    void setID( const rtl::OUString& );  /// set ID for this submission

    rtl::OUString getBind() const;
    void setBind( const rtl::OUString& );

    rtl::OUString getRef() const;
    void setRef( const rtl::OUString& );

    rtl::OUString getAction() const;
    void setAction( const rtl::OUString& );

    rtl::OUString getMethod() const;
    void setMethod( const rtl::OUString& );

    rtl::OUString getVersion() const;
    void setVersion( const rtl::OUString& );

    bool getIndent() const;
    void setIndent( bool );

    rtl::OUString getMediaType() const;
    void setMediaType( const rtl::OUString& );

    rtl::OUString getEncoding() const;
    void setEncoding( const rtl::OUString& );

    bool getOmitXmlDeclaration() const;
    void setOmitXmlDeclaration( bool );

    bool getStandalone() const;
    void setStandalone( bool );

    rtl::OUString getCDataSectionElement() const;
    void setCDataSectionElement( const rtl::OUString& );

    rtl::OUString getReplace() const;
    void setReplace( const rtl::OUString& );

    rtl::OUString getSeparator() const;
    void setSeparator( const rtl::OUString& );

    com::sun::star::uno::Sequence< rtl::OUString > getIncludeNamespacePrefixes() const;
    void setIncludeNamespacePrefixes( const com::sun::star::uno::Sequence< rtl::OUString >& );


    /** perform the submission
     * @returns if submission was successful */
    bool doSubmit( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& aHandler );


    /// release the model (note: Binding is unusable without model)
    void releaseModel();


    // helpers for UNO tunnel
    static com::sun::star::uno::Sequence<sal_Int8> getUnoTunnelID();
    static Submission* getSubmission( const com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& );



private:

    /// check whether object is live, and throw suitable exception if not
    /// (to be used be API methods before acting on the object)
    void liveCheck()
        throw( com::sun::star::uno::RuntimeException );

    /// get the model implementation
    xforms::Model* getModelImpl() const;
    xforms::Model* getModelImpl( const com::sun::star::uno::Reference<com::sun::star::xforms::XModel>& xModel ) const;



protected:

    //
    // XPropertySet & friends:
    //   implement abstract methods from PropertySetHelper
    //

    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        com::sun::star::uno::Any& rConvertedValue,
        com::sun::star::uno::Any& rOldValue,
        sal_Int32 nHandle,
        const com::sun::star::uno::Any& rValue )
        throw ( com::sun::star::lang::IllegalArgumentException );

private:
    void initializePropertySet();


public:

    //
    // XNamed:
    //   get/set name
    //

    virtual rtl::OUString SAL_CALL getName()
        throw( com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setName( const rtl::OUString& )
        throw( com::sun::star::uno::RuntimeException );



    //
    // XUnoTunnel
    //

    virtual sal_Int64 SAL_CALL getSomething(
        const com::sun::star::uno::Sequence<sal_Int8>& )
        throw( com::sun::star::uno::RuntimeException );


    //
    // XSubmission
    //

    virtual void SAL_CALL submit(  )
        throw ( com::sun::star::util::VetoException,
                com::sun::star::lang::WrappedTargetException,
                com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL submitWithInteraction(
        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& aHandler )
        throw ( com::sun::star::util::VetoException,
                com::sun::star::lang::WrappedTargetException,
                com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addSubmissionVetoListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::form::submission::XSubmissionVetoListener >& listener )
        throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeSubmissionVetoListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::form::submission::XSubmissionVetoListener >& listener )
        throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

    //
    // XPropertySet
    // (need to disambiguate this)

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
};


} // namespace xforms

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
