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

#include <stdio.h>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <resourcemodel/TableManager.hxx>
#include <resourcemodel/QNameToString.hxx>
#include <resourcemodel/exceptions.hxx>
#include <resourcemodel/SubSequence.hxx>
#include <resourcemodel/util.hxx>
#include <resourcemodel.hxx>

namespace writerfilter {

class ResourceModelOutputWithDepth : public OutputWithDepth<string>
{
public:
    ResourceModelOutputWithDepth()
    : OutputWithDepth<string>("<tablegroup>", "</tablegroup>") {}

    ~ResourceModelOutputWithDepth() {outputGroup();}

    void output(const string & str) const { cout << str << endl; }
};

ResourceModelOutputWithDepth output;

// ----- WW8TableDataManager -------------------------------

void WW8BinaryObjHandler::data
(const sal_uInt8 * buf, size_t length,
 writerfilter::Reference<Properties>::Pointer_t /*pRef*/)
{
    SubSequence<sal_uInt8> aSeq(buf, length);

    aSeq.dump(output);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
