/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _BINDING_HXX
#define _BINDING_HXX

#include <com/sun/star/uno/Reference.hxx>

// forward declaractions
namespace xforms
{
    class Model;
    class EvaluationContext;
}
namespace com { namespace sun { namespace star {
    namespace xml {
        namespace xpath { class XXPathAPI; }
        namespace dom
        {
            class XNode;
            class XNodeList;
        }
    }
    namespace container { class XNameContainer; }
    namespace xforms { class XModel; }
    namespace xsd { class XDataType; }
} } }

#include <cppuhelper/implbase8.hxx>
#include <propertysetbase.hxx>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/form/binding/XListEntrySource.hpp>
#include <com/sun/star/form/validation/XValidator.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/xml/dom/events/XEventListener.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/util/XCloneable.hpp>

#include "pathexpression.hxx"
#include "boolexpression.hxx"
#include "mip.hxx"
#include <rtl/ustring.hxx>
#include <vector>
#include <memory> // auto_ptr



namespace xforms
{

/** An XForms Binding. Contains:
 *  # a connection to its model
 *  # an ID
 *  # an binding expression
 *  # model item properties
 *  # (NOT YET IMPLEMENTED) child bindings (sequence of)
 *
 * See http://www.w3.org/TR/xforms/ for more information.
 */

typedef cppu::ImplInheritanceHelper8<
    PropertySetBase,
    com::sun::star::form::binding::XValueBinding,
    com::sun::star::form::binding::XListEntrySource,
    com::sun::star::form::validation::XValidator,
    com::sun::star::util::XModifyBroadcaster,
    com::sun::star::container::XNamed,
    com::sun::star::xml::dom::events::XEventListener,
    com::sun::star::lang::XUnoTunnel,
    com::sun::star::util::XCloneable
> Binding_t;

class Binding : public Binding_t
{
public:
    typedef com::sun::star::uno::Reference<com::sun::star::xforms::XModel> Model_t;
    typedef com::sun::star::uno::Reference<com::sun::star::util::XModifyListener> XModifyListener_t;
    typedef std::vector<XModifyListener_t> ModifyListeners_t;
    typedef com::sun::star::uno::Reference<com::sun::star::form::validation::XValidityConstraintListener> XValidityConstraintListener_t;
    typedef std::vector<XValidityConstraintListener_t> XValidityConstraintListeners_t;
    typedef com::sun::star::uno::Reference<com::sun::star::form::binding::XListEntryListener> XListEntryListener_t;
    typedef std::vector<XListEntryListener_t> XListEntryListeners_t;
    typedef com::sun::star::uno::Reference<com::sun::star::container::XNameContainer> XNameContainer_t;
    typedef com::sun::star::uno::Reference<com::sun::star::xml::dom::XNode> XNode_t;
    typedef com::sun::star::uno::Reference<com::sun::star::xml::dom::XNodeList> XNodeList_t;
    typedef com::sun::star::uno::Reference<com::sun::star::util::XCloneable> XCloneable_t;
    typedef com::sun::star::uno::Sequence<sal_Int8> IntSequence_t;
    typedef com::sun::star::uno::Sequence<rtl::OUString> StringSequence_t;
    typedef std::vector<MIP> MIPs_t;
    typedef std::vector<XNode_t> XNodes_t;



private:

    /// the Model to which this Binding belongs; may be NULL
    Model_t mxModel;

    /// binding-ID. A document-wide unique ID for this binding element.
    rtl::OUString msBindingID;

    /// an XPath-expression to be instantiated on the data instance
    PathExpression maBindingExpression;

    /// an XPath-expression to determine read-only status
    BoolExpression maReadonly;

    /// an XPath-expression to determine relevance
    BoolExpression maRelevant;

    /// an XPath-expression to determine if item is required
    BoolExpression maRequired;

    /// an XPath-expression to determine if item is valid
    BoolExpression maConstraint;

    /// user-readable explanation of the constraint
    rtl::OUString msExplainConstraint;

    /// an XPath-expression to calculate values
    ComputedExpression maCalculate;

    /// the XML namespaces used for XML names/XPath-expressions in this binding
    XNameContainer_t mxNamespaces;

    /// a type name
    rtl::OUString msTypeName;

    /// modify listeners
    ModifyListeners_t maModifyListeners;

    /// list entry listener
    XListEntryListeners_t maListEntryListeners;

    /// validity listeners;
    XValidityConstraintListeners_t maValidityListeners;

    /// nodes on which we are listening for events
    XNodes_t maEventNodes;

    /// the current MIP object for the first node we are bound to
    MIP maMIP;

    /// flag to detect recursions in calculate
    bool mbInCalculate;

    // flags to manage deferred notifications:
    /// if >0, valueModified() and bindingModified() will only set flags
    sal_Int32 mnDeferModifyNotifications;
    bool mbValueModified;   /// if true, valueModified needs to be called
    bool mbBindingModified; /// if true, bindingModified needs to be called


    void initializePropertySet();


public:
    Binding();
    virtual ~Binding() throw();

    //
    // property methods: get/set value
    //

    Model_t getModel() const;   /// get XForms model
    void _setModel( const Model_t& ); /// set XForms model (only called by Model)


    rtl::OUString getModelID() const;   /// get ID of XForms model

    rtl::OUString getBindingID() const;         /// get ID for this binding
    void setBindingID( const rtl::OUString& );  /// set ID for this binding

    rtl::OUString getBindingExpression() const; /// get binding expression
    void setBindingExpression( const rtl::OUString& );  /// set binding exp.

    // MIPs (model item properties)

    rtl::OUString getReadonlyExpression() const;         /// get read-only MIP
    void setReadonlyExpression( const rtl::OUString& );  /// set read-only MIP

    rtl::OUString getRelevantExpression() const;         /// get relevant MIP
    void setRelevantExpression( const rtl::OUString& );  /// set relevant MIP

    rtl::OUString getRequiredExpression() const;         /// get required MIP
    void setRequiredExpression( const rtl::OUString& );  /// set required MIP

    rtl::OUString getConstraintExpression() const;       /// get constraint MIP
    void setConstraintExpression( const rtl::OUString& );/// set constraint MIP

    rtl::OUString getCalculateExpression() const;        /// get calculate MIP
    void setCalculateExpression( const rtl::OUString& ); /// set calculate MIP

    rtl::OUString getType() const;         /// get type name MIP (static)
    void setType( const rtl::OUString& );  /// set type name MIP (static)

    // a binding expression can only be interpreted with respect to
    // suitable namespace declarations. We collect those in the model and in a binding.

    // access to a binding's namespace
    // (set-method only changes local namespaces (but may add to model))
    XNameContainer_t getBindingNamespaces() const;  /// set binding namespaces
    void setBindingNamespaces( const XNameContainer_t& ); /// get binding nmsp.

    // access to the model's namespaces
    // (set-method changes model's namespaces (unless a local one is present))
    XNameContainer_t getModelNamespaces() const;  /// set model namespaces
    void setModelNamespaces( const XNameContainer_t& ); /// get model nmsp.


    // read-only properties that map MIPs to control data source properties
    bool getReadOnly() const;       // MIP readonly
    bool getRelevant() const;       // MIP relevant
    bool getExternalData() const;   // mapped from model's ExternalData property


    // missing binding properties:
    // - type (static; default: xsd:string)
    // - minOccurs/maxOccurs (computed XPath; default: 0/inf)
    // - p3ptype (static; no default)




    /// get this binding's context node
    xforms::EvaluationContext getEvaluationContext() const;

    /// get evalation contexts for this binding's MIPs
    std::vector<xforms::EvaluationContext> getMIPEvaluationContexts();

    /// get nodeset the bind is bound to
    XNodeList_t getXNodeList();

    /// heuristically determine whether this binding is simple binding
    /// (here: simple binding == does not depend on other parts of the
    ///                          instance, it's not a 'dynamic' binding)
    bool isSimpleBinding() const;

    /// heuristically determine whether this binding's binding
    /// expression is simple
    bool isSimpleBindingExpression() const;

    /// update this binding (e.g. called by model for refresh )
    void update();

    /// prevent change notifications being sent to controls
    void deferNotifications( bool );

    /// is this binding valid? (are constraint, type and required MIPs ok?)
    bool isValid();

    /// determine whether this binding currently performs a useful
    /// function, r whether is may be discarded
    bool isUseful();

    /// explain why binding is invalid
    rtl::OUString explainInvalid();


    // the ID for XUnoTunnel calls
    static IntSequence_t getUnoTunnelID();
    static Binding* getBinding( const com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& );

    //
    // class-scoped typedef for easy-to-read UNO interfaces
    //

    // basic types
    typedef com::sun::star::uno::Any Any_t;
    typedef com::sun::star::uno::Sequence<com::sun::star::uno::Type> Sequence_Type_t;
    typedef com::sun::star::uno::Type Type_t;

    // reference types
    typedef com::sun::star::uno::Reference<com::sun::star::beans::XPropertyChangeListener> XPropertyChangeListener_t;
    typedef com::sun::star::uno::Reference<com::sun::star::beans::XPropertySetInfo> XPropertySetInfo_t;
    typedef com::sun::star::uno::Reference<com::sun::star::beans::XVetoableChangeListener> XVetoableChangeListener_t;
    typedef com::sun::star::uno::Reference<com::sun::star::xml::xpath::XXPathAPI> XXPathAPI_t;
    typedef com::sun::star::uno::Reference<com::sun::star::xml::dom::events::XEvent> XEvent_t;
    typedef com::sun::star::uno::Reference<com::sun::star::xsd::XDataType> XDataType_t;

    // exceptions
    typedef com::sun::star::beans::PropertyVetoException PropertyVetoException_t;
    typedef com::sun::star::beans::UnknownPropertyException UnknownPropertyException_t;
    typedef com::sun::star::lang::IllegalArgumentException IllegalArgumentException_t;
    typedef com::sun::star::lang::NoSupportException NoSupportException_t;
    typedef com::sun::star::lang::WrappedTargetException WrappedTargetException_t;
    typedef com::sun::star::uno::RuntimeException RuntimeException_t;
    typedef com::sun::star::form::binding::IncompatibleTypesException IncompatibleTypesException_t;
    typedef com::sun::star::form::binding::InvalidBindingStateException InvalidBindingStateException_t;
    typedef com::sun::star::lang::NullPointerException NullPointerException_t;
    typedef com::sun::star::lang::IndexOutOfBoundsException IndexOutOfBoundsException_t;



private:
    /// check whether object is live, and throw suitable exception if not
    /// (to be used be API methods before acting on the object)
    void checkLive() throw( RuntimeException_t );

    /// check whether binding has a model, and throw exception if not
    /// (to be used be API methods before acting on the object)
    void checkModel() throw( RuntimeException_t );

    /// determine whether object is live
    /// live: has model, and model has been initialized
    bool isLive() const;

    /// get the model implementation
    xforms::Model* getModelImpl() const;
    xforms::Model* getModelImpl( const Model_t& xModel ) const;

    /// get MIP evaluation contexts
    /// (only valid if control has already been bound)
    std::vector<xforms::EvaluationContext> _getMIPEvaluationContexts() const;

    /// bind this binding, and pre-compute the affected nodes
    void bind( bool bForceRebind = false );

    /// the binding value has been changed:
    ///   trigger a modified event on all modified listeners
    void valueModified();

    /// the binding itself has changed:
    ///   force rebind, then call valueModified()
    void bindingModified();


    /// register the event listeners for
    void registerListeners();

    /// set MIPs defined by this binding on MIP item
    MIP getLocalMIP() const;

    /// get the data type that applies to this binding
    XDataType_t getDataType();

    /// determine whether binding is valid according to the given data type
    bool isValid_DataType();

    /// explain validity of binding with respect to the given data type
    rtl::OUString explainInvalid_DataType();

    /// 'clear' this binding - remove all listeners, etc.
    void clear();

    /// distribute MIPs from current node recursively to children
    void distributeMIP( const XNode_t &rxNode );

    /// implement get*Namespaces()
    XNameContainer_t _getNamespaces() const;

    /// implement set*Namespaces()
    void _setNamespaces( const XNameContainer_t&, bool bBinding );

    /// set a useful default binding ID (if none is set)
    void _checkBindingID();

public:
    /// for debugging purposes only: get the MIPs defined by this binding
    const MIP* _getMIP();





    //
    // XValueBinding:
    //

public:

    virtual Sequence_Type_t SAL_CALL getSupportedValueTypes()
        throw( RuntimeException_t );

    virtual sal_Bool SAL_CALL supportsType( const Type_t& aType )
        throw( RuntimeException_t );

    virtual Any_t SAL_CALL getValue( const Type_t& aType )
        throw( IncompatibleTypesException_t,
               RuntimeException_t );

    virtual void SAL_CALL setValue( const Any_t& aValue )
        throw( IncompatibleTypesException_t,
               InvalidBindingStateException_t,
               NoSupportException_t,
               RuntimeException_t );



    //
    // XListEntry Source
    //

    virtual sal_Int32 SAL_CALL getListEntryCount()
        throw( RuntimeException_t );

    virtual rtl::OUString SAL_CALL getListEntry( sal_Int32 nPosition )
        throw( IndexOutOfBoundsException_t,
               RuntimeException_t );

    virtual StringSequence_t SAL_CALL getAllListEntries()
        throw( RuntimeException_t );

    virtual void SAL_CALL addListEntryListener( const XListEntryListener_t& )
        throw( NullPointerException_t,
               RuntimeException_t );

    virtual void SAL_CALL removeListEntryListener( const XListEntryListener_t&)
        throw( NullPointerException_t,
               RuntimeException_t );



    //
    // XValidator:
    //

    virtual sal_Bool SAL_CALL isValid(
        const Any_t& )
        throw( RuntimeException_t );

    virtual rtl::OUString SAL_CALL explainInvalid(
        const Any_t& )
        throw( RuntimeException_t );

    virtual void SAL_CALL addValidityConstraintListener(
        const XValidityConstraintListener_t& xListener )
        throw( NullPointerException_t,
               RuntimeException_t );

    virtual void SAL_CALL removeValidityConstraintListener(
        const XValidityConstraintListener_t& xListener )
        throw( NullPointerException_t,
               RuntimeException_t );


    //
    // XModifyBroadcaster & friends:
    //   inform listeners about changes in our values
    //

public:

    virtual void SAL_CALL addModifyListener(
        const XModifyListener_t& xListener )
        throw( RuntimeException_t );

    virtual void SAL_CALL removeModifyListener(
        const XModifyListener_t& xListener )
        throw( RuntimeException_t );




    //
    // XNamed:
    //   get/set name
    //

public:

    virtual rtl::OUString SAL_CALL getName()
        throw( RuntimeException_t );

    virtual void SAL_CALL setName( const rtl::OUString& )
        throw( RuntimeException_t );



    //
    // xml::dom::event::XEventListener
    //   receive an event if our node changed
    //

    virtual void SAL_CALL handleEvent(
        const XEvent_t& xEvent )
        throw( RuntimeException_t );



    //
    // XUnoTunnel
    //

    virtual sal_Int64 SAL_CALL getSomething( const IntSequence_t& )
        throw( RuntimeException_t );


    //
    // XCloneable
    //

    virtual XCloneable_t SAL_CALL createClone()
        throw( RuntimeException_t );
};


} // namespace xforms

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
