/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: resourcemodel.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 12:17:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include <resourcemodel/WW8ResourceModel.hxx>

namespace writerfilter {
class WW8StreamHandler : public Stream
{
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

    static bool compare(SprmSharedPointer_t sprm1, SprmSharedPointer_t sprm2);
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
