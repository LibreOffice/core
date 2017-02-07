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

#ifndef INCLUDED_FORMS_SOURCE_XFORMS_BINDING_HXX
#define INCLUDED_FORMS_SOURCE_XFORMS_BINDING_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/implbase.hxx>
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

typedef cppu::ImplInheritanceHelper<
    PropertySetBase,
    css::form::binding::XValueBinding,
    css::form::binding::XListEntrySource,
    css::form::validation::XValidator,
    css::util::XModifyBroadcaster,
    css::container::XNamed,
    css::xml::dom::events::XEventListener,
    css::lang::XUnoTunnel,
    css::util::XCloneable
> Binding_t;

class Binding : public Binding_t
{
public:
    typedef std::vector<css::uno::Reference<css::util::XModifyListener> > ModifyListeners_t;
    typedef std::vector<css::uno::Reference<css::form::validation::XValidityConstraintListener> > XValidityConstraintListeners_t;
    typedef std::vector<css::uno::Reference<css::form::binding::XListEntryListener> > XListEntryListeners_t;


private:

    /// the Model to which this Binding belongs; may be NULL
    css::uno::Reference<css::xforms::XModel> mxModel;

    /// binding-ID. A document-wide unique ID for this binding element.
    OUString msBindingID;

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
    OUString msExplainConstraint;

    /// an XPath-expression to calculate values
    ComputedExpression maCalculate;

    /// the XML namespaces used for XML names/XPath-expressions in this binding
    css::uno::Reference<css::container::XNameContainer> mxNamespaces;

    /// a type name
    OUString msTypeName;

    /// modify listeners
    ModifyListeners_t maModifyListeners;

    /// list entry listener
    XListEntryListeners_t maListEntryListeners;

    /// validity listeners;
    XValidityConstraintListeners_t maValidityListeners;

    /// nodes on which we are listening for events
    std::vector<css::uno::Reference<css::xml::dom::XNode> > maEventNodes;

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
    virtual ~Binding() override;


    // property methods: get/set value


    css::uno::Reference<css::xforms::XModel> getModel() const { return mxModel;}   /// get XForms model
    void _setModel( const css::uno::Reference<css::xforms::XModel>& ); /// set XForms model (only called by Model)


    OUString getModelID() const;   /// get ID of XForms model

    OUString getBindingID() const { return msBindingID;}         /// get ID for this binding
    void setBindingID( const OUString& );  /// set ID for this binding

    OUString getBindingExpression() const; /// get binding expression
    void setBindingExpression( const OUString& );  /// set binding exp.

    // MIPs (model item properties)

    OUString getReadonlyExpression() const;         /// get read-only MIP
    void setReadonlyExpression( const OUString& );  /// set read-only MIP

    OUString getRelevantExpression() const;         /// get relevant MIP
    void setRelevantExpression( const OUString& );  /// set relevant MIP

    OUString getRequiredExpression() const;         /// get required MIP
    void setRequiredExpression( const OUString& );  /// set required MIP

    OUString getConstraintExpression() const;       /// get constraint MIP
    void setConstraintExpression( const OUString& );/// set constraint MIP

    OUString getCalculateExpression() const;        /// get calculate MIP
    void setCalculateExpression( const OUString& ); /// set calculate MIP

    OUString getType() const { return msTypeName;}         /// get type name MIP (static)
    void setType( const OUString& );  /// set type name MIP (static)

    // a binding expression can only be interpreted with respect to
    // suitable namespace declarations. We collect those in the model and in a binding.

    // access to a binding's namespace
    // (set-method only changes local namespaces (but may add to model))
    css::uno::Reference<css::container::XNameContainer> getBindingNamespaces() const { return mxNamespaces; }
    void setBindingNamespaces( const css::uno::Reference<css::container::XNameContainer>& ); /// get binding nmsp.

    // access to the model's namespaces
    // (set-method changes model's namespaces (unless a local one is present))
    css::uno::Reference<css::container::XNameContainer> getModelNamespaces() const;  /// set model namespaces
    void setModelNamespaces( const css::uno::Reference<css::container::XNameContainer>& ); /// get model nmsp.


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
    css::uno::Reference<css::xml::dom::XNodeList> getXNodeList();

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
    OUString explainInvalid();


    // the ID for XUnoTunnel calls
    static css::uno::Sequence<sal_Int8> getUnoTunnelID();
    static Binding* getBinding( const css::uno::Reference<css::beans::XPropertySet>& );


private:
    /// check whether object is live, and throw suitable exception if not
    /// (to be used be API methods before acting on the object)
    ///
    /// @throws css::uno::RuntimeException
    void checkLive();

    /// determine whether object is live
    /// live: has model, and model has been initialized
    bool isLive() const;

    /// get the model implementation
    xforms::Model* getModelImpl() const;
    static xforms::Model* getModelImpl( const css::uno::Reference<css::xforms::XModel>& xModel );

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


    /// set MIPs defined by this binding on MIP item
    MIP getLocalMIP() const;

    /// get the data type that applies to this binding
    css::uno::Reference<css::xsd::XDataType> getDataType();

    /// determine whether binding is valid according to the given data type
    bool isValid_DataType();

    /// explain validity of binding with respect to the given data type
    OUString explainInvalid_DataType();

    /// 'clear' this binding - remove all listeners, etc.
    void clear();

    /// distribute MIPs from current node recursively to children
    void distributeMIP( const css::uno::Reference<css::xml::dom::XNode> &rxNode );

    /// implement get*Namespaces()
    css::uno::Reference<css::container::XNameContainer> _getNamespaces() const;

    /// implement set*Namespaces()
    void _setNamespaces( const css::uno::Reference<css::container::XNameContainer>&, bool bBinding );

    /// set a useful default binding ID (if none is set)
    void _checkBindingID();

public:

    virtual css::uno::Sequence<css::uno::Type> SAL_CALL getSupportedValueTypes() override;

    virtual sal_Bool SAL_CALL supportsType( const css::uno::Type& aType ) override;

    virtual css::uno::Any SAL_CALL getValue( const css::uno::Type& aType ) override;

    virtual void SAL_CALL setValue( const css::uno::Any& aValue ) override;


    // XListEntrySource


    virtual sal_Int32 SAL_CALL getListEntryCount() override;

    virtual OUString SAL_CALL getListEntry( sal_Int32 nPosition ) override;

    virtual css::uno::Sequence<OUString> SAL_CALL getAllListEntries() override;

    virtual void SAL_CALL addListEntryListener( const css::uno::Reference<css::form::binding::XListEntryListener>& ) override;

    virtual void SAL_CALL removeListEntryListener( const css::uno::Reference<css::form::binding::XListEntryListener>&) override;


    // XValidator:


    virtual sal_Bool SAL_CALL isValid(
        const css::uno::Any& ) override;

    virtual OUString SAL_CALL explainInvalid(
        const css::uno::Any& ) override;

    virtual void SAL_CALL addValidityConstraintListener(
        const css::uno::Reference<css::form::validation::XValidityConstraintListener>& xListener ) override;

    virtual void SAL_CALL removeValidityConstraintListener(
        const css::uno::Reference<css::form::validation::XValidityConstraintListener>& xListener ) override;


    // XModifyBroadcaster & friends:
    //   inform listeners about changes in our values


public:

    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference<css::util::XModifyListener>& xListener ) override;

    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference<css::util::XModifyListener>& xListener ) override;


    // XNamed:
    //   get/set name


public:

    virtual OUString SAL_CALL getName() override;

    virtual void SAL_CALL setName( const OUString& ) override;


    // xml::dom::event::XEventListener
    //   receive an event if our node changed


    virtual void SAL_CALL handleEvent(
        const css::uno::Reference<css::xml::dom::events::XEvent>& xEvent ) override;


    // XUnoTunnel


    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence<sal_Int8>& ) override;


    // XCloneable


    virtual css::uno::Reference<css::util::XCloneable> SAL_CALL createClone() override;
};


} // namespace xforms

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
