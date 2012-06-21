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

#ifndef INCLUDED_DFF_HXX
#define INCLUDED_DFF_HXX

#include <vector>
#include "WW8StructBase.hxx"
#include "WW8ResourceModelImpl.hxx"

namespace writerfilter {
namespace doctok
{
using std::vector;

class DffBlock;

class DffRecord : public WW8StructBase, public writerfilter::Reference<Properties>,
                  public Sprm
{
    bool bInitialized;
public:
    typedef boost::shared_ptr<DffRecord> Pointer_t;

protected:
    typedef vector<Pointer_t> Records_t;
    Records_t mRecords;

    void initChildren();

public:

    DffRecord(WW8Stream & rStream, sal_uInt32 nOffset, sal_uInt32 nCount);
    DffRecord(WW8StructBase * pParent, sal_uInt32 nOffset, sal_uInt32 nCount);
    virtual ~DffRecord() {}

    bool isContainer() const;
    sal_uInt32 calcSize() const;

    sal_uInt32 getVersion() const;
    sal_uInt32 getInstance() const;
    sal_uInt32 getRecordType() const;

    virtual DffRecord * clone() const { return new DffRecord(*this); }

    virtual void resolveLocal(Properties & rHandler);
    virtual void resolveChildren(Properties & rHandler);

    Records_t findRecords(sal_uInt32 nType, bool bRecursive = true,
                          bool bAny = false);

    void findRecords
    (sal_uInt32 nType, Records_t & rRecords,
     bool bRecursive = true, bool bAny = false);

    Records_t::iterator begin();
    Records_t::iterator end();

    sal_uInt32 getShapeType();
    sal_uInt32 getShapeId();
    sal_uInt32 getShapeBid();

    /* Properties methods */
    virtual void resolve(Properties & rHandler);
    virtual string getType() const;

    /* Sprm methods */
    virtual sal_uInt32 getId() const { return getRecordType(); }
    virtual Value::Pointer_t getValue();
    virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary();
    virtual writerfilter::Reference<Stream>::Pointer_t getStream();
    virtual writerfilter::Reference<Properties>::Pointer_t getProps();

    virtual string toString() const;
    virtual string getName() const;

    virtual Kind getKind();

    friend class DffBlock;
};

typedef vector<DffRecord::Pointer_t> Records_t;

class DffBlock : public WW8StructBase,
                 public writerfilter::Reference<Properties>
{
    bool bInitialized;
    sal_uInt32 mnPadding;

    Records_t mRecords;

protected:
    void initChildren();

public:
    typedef boost::shared_ptr<DffBlock> Pointer_t;

    DffBlock(WW8Stream & rStream, sal_uInt32 nOffset, sal_uInt32 nCount, sal_uInt32 nPadding);
    DffBlock(WW8StructBase * pParent, sal_uInt32 nOffset, sal_uInt32 nCount, sal_uInt32 nPadding);
    DffBlock(const DffBlock & rSrc);
    virtual ~DffBlock() {}

    Records_t findRecords(sal_uInt32 nType, bool bRecursive = true,
                          bool bAny = false);

    void findRecords(sal_uInt32 nType, Records_t & rRecords,
                     bool bRecursive = true, bool bAny = false);

    DffRecord::Pointer_t getShape(sal_uInt32 nSpid);
    DffRecord::Pointer_t getBlip(sal_uInt32 nBlip);

    Records_t::iterator begin();
    Records_t::iterator end();

    /* Properties methods */
    virtual void resolve(Properties & rHandler);
    virtual string getType() const;
};

DffRecord *
createDffRecord(WW8StructBase * pParent, sal_uInt32 nOffset,
                sal_uInt32 * nSize = NULL);
DffRecord *
createDffRecord(WW8Stream & rStream, sal_uInt32 nOffset,
                sal_uInt32 * nSize = NULL);

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
