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
#include <resourcemodel/WW8ResourceModel.hxx>

namespace writerfilter {
class WW8TableManager;
class WW8StreamHandler : public Stream
{
    int mnUTextCount;
    WW8TableManager* mpTableManager;

public:
    WW8StreamHandler();
    virtual ~WW8StreamHandler();

    virtual void startSectionGroup();
    virtual void endSectionGroup();
    virtual void startParagraphGroup();
    virtual void endParagraphGroup();
    virtual void startCharacterGroup();
    virtual void endCharacterGroup();
    virtual void text(const sal_uInt8 * data, size_t len);
    virtual void utext(const sal_uInt8 * data, size_t len);

    virtual void props(writerfilter::Reference<Properties>::Pointer_t ref);
    virtual void table(Id name,
                       writerfilter::Reference<Table>::Pointer_t ref);

    virtual void startShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape );
    virtual void endShape( );

    virtual void substream(Id name, writerfilter::Reference<Stream>::Pointer_t ref);

    virtual void info(const string & info);
};

class WW8PropertiesHandler : public Properties
{
    typedef boost::shared_ptr<Sprm> SprmSharedPointer_t;
    typedef vector<SprmSharedPointer_t> SprmPointers_t;
    SprmPointers_t sprms;
    WW8TableManager* mpTableManager;

public:
    WW8PropertiesHandler(WW8TableManager* pTableManager)
        : mpTableManager(pTableManager)
    {
    }

    virtual ~WW8PropertiesHandler()
    {
    }

    virtual void attribute(Id name, Value & val);
    virtual void sprm(Sprm & sprm);

    void dumpSprm(SprmSharedPointer_t sprm);
    void dumpSprms();
};

class WW8BinaryObjHandler : public BinaryObj
{
public:
    WW8BinaryObjHandler()
    {
    }

    virtual ~WW8BinaryObjHandler()
    {
    }

    virtual void data(const sal_uInt8* buf, size_t len,
                      writerfilter::Reference<Properties>::Pointer_t ref);
};

class WW8TableHandler : public Table
{
    WW8TableManager* mpTableManager;

public:
    WW8TableHandler(WW8TableManager* pTableManager)
        : mpTableManager(pTableManager)
    {
    }

    virtual ~WW8TableHandler()
    {
    }

    void entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref);
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
