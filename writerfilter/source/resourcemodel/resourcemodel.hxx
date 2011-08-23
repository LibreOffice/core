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
#include <resourcemodel/WW8ResourceModel.hxx>

namespace writerfilter {
class WW8StreamHandler : public Stream
{    
    int mnUTextCount;
    
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

public:
    WW8PropertiesHandler()
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
public:
    WW8TableHandler()
    {
    }

    virtual ~WW8TableHandler()
    {
    }

    void entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref);    
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
