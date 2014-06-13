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

#ifndef INCLUDED_FORMS_SOURCE_XFORMS_SUBMISSION_HXX
#define INCLUDED_FORMS_SOURCE_XFORMS_SUBMISSION_HXX

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
    OUString msID;
    OUString msBind;
    ComputedExpression maRef;
    OUString msAction;
    OUString msMethod;
    OUString msVersion;
    bool mbIndent;
    OUString msMediaType;
    OUString msEncoding;
    bool mbOmitXmlDeclaration;
    bool mbStandalone;
    OUString msCDataSectionElement;
    OUString msReplace;
    OUString msSeparator;
    com::sun::star::uno::Sequence< OUString > msIncludeNamespacePrefixes;

private:

    /// the Model to which this Submission belongs; may be NULL
    com::sun::star::uno::Reference<com::sun::star::xforms::XModel> mxModel;

    // this will extract the document from the model that will be submitted
    com::sun::star::uno::Reference< com::sun::star::xml::dom::XDocumentFragment >
        createSubmissionDocument(const com::sun::star::uno::Reference< com::sun::star::xml::xpath::XXPathObject >& aObject,
                                 bool bRemoveWSNodes = false);
    com::sun::star::uno::Reference< com::sun::star::xml::dom::XDocument >
        getInstanceDocument(const com::sun::star::uno::Reference< com::sun::star::xml::xpath::XXPathObject >& aObject);

public:
    Submission();
    virtual ~Submission() throw();


    // property methods: get/set value


    /// get XForms model
    com::sun::star::uno::Reference<com::sun::star::xforms::XModel>
        getModel() const { return mxModel;}

    /// set XForms model
    void setModel(
        const com::sun::star::uno::Reference<com::sun::star::xforms::XModel>& );

    OUString getID() const { return msID;}         /// get ID for this submission
    void setID( const OUString& );  /// set ID for this submission

    OUString getBind() const { return msBind;}
    void setBind( const OUString& );

    OUString getRef() const;
    void setRef( const OUString& );

    OUString getAction() const { return msAction;}
    void setAction( const OUString& );

    OUString getMethod() const { return msMethod;}
    void setMethod( const OUString& );

    OUString getVersion() const { return msVersion;}
    void setVersion( const OUString& );

    bool getIndent() const { return mbIndent;}
    void setIndent( bool );

    OUString getMediaType() const { return msMediaType;}
    void setMediaType( const OUString& );

    OUString getEncoding() const { return msEncoding;}
    void setEncoding( const OUString& );

    bool getOmitXmlDeclaration() const { return mbOmitXmlDeclaration;}
    void setOmitXmlDeclaration( bool );

    bool getStandalone() const { return mbStandalone;}
    void setStandalone( bool );

    OUString getCDataSectionElement() const { return msCDataSectionElement;}
    void setCDataSectionElement( const OUString& );

    OUString getReplace() const { return msReplace;}
    void setReplace( const OUString& );

    OUString getSeparator() const { return msSeparator;}
    void setSeparator( const OUString& );

    com::sun::star::uno::Sequence< OUString > getIncludeNamespacePrefixes() const { return msIncludeNamespacePrefixes;}
    void setIncludeNamespacePrefixes( const com::sun::star::uno::Sequence< OUString >& );


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


    // XPropertySet & friends:
    //   implement abstract methods from PropertySetHelper


    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        com::sun::star::uno::Any& rConvertedValue,
        com::sun::star::uno::Any& rOldValue,
        sal_Int32 nHandle,
        const com::sun::star::uno::Any& rValue )
        throw ( com::sun::star::lang::IllegalArgumentException ) SAL_OVERRIDE;

private:
    void initializePropertySet();


public:


    // XNamed:
    //   get/set name


    virtual OUString SAL_CALL getName()
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL setName( const OUString& )
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;




    // XUnoTunnel


    virtual sal_Int64 SAL_CALL getSomething(
        const com::sun::star::uno::Sequence<sal_Int8>& )
        throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;



    // XSubmission


    virtual void SAL_CALL submit(  )
        throw ( com::sun::star::util::VetoException,
                com::sun::star::lang::WrappedTargetException,
                com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL submitWithInteraction(
        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& aHandler )
        throw ( com::sun::star::util::VetoException,
                com::sun::star::lang::WrappedTargetException,
                com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL addSubmissionVetoListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::form::submission::XSubmissionVetoListener >& listener )
        throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeSubmissionVetoListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::form::submission::XSubmissionVetoListener >& listener )
        throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;


    // XPropertySet
    // (need to disambiguate this)

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


} // namespace xforms

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
