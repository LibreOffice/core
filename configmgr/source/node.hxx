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

#ifndef INCLUDED_CONFIGMGR_SOURCE_NODE_HXX
#define INCLUDED_CONFIGMGR_SOURCE_NODE_HXX

#include <sal/config.h>

#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>

namespace configmgr {

class NodeMap;

class Node: public salhelper::SimpleReferenceObject {
public:
    enum Kind {
        KIND_PROPERTY, KIND_LOCALIZED_PROPERTY, KIND_LOCALIZED_VALUE,
        KIND_GROUP, KIND_SET, KIND_ROOT };

    virtual Kind kind() const = 0;

    virtual rtl::Reference< Node > clone(bool keepTemplateName) const = 0;

    virtual NodeMap & getMembers();
    virtual OUString getTemplateName() const;

    virtual void setMandatory(int layer);
    virtual int getMandatory() const;

    void setLayer(int layer);
    int getLayer() const { return layer_;}

    void setFinalized(int layer);
    int getFinalized() const { return finalized_;}

    rtl::Reference< Node > getMember(OUString const & name);

protected:
    explicit Node(int layer);
    explicit Node(const Node & other);

    virtual ~Node() override;
private:
    int layer_;
    int finalized_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
