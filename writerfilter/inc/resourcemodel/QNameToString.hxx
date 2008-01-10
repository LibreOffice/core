/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: QNameToString.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:32:56 $
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
#ifndef INCLUDED_QNAME_TO_STRING_HXX
#define INCLUDED_QNAME_TO_STRING_HXX

#include <boost/shared_ptr.hpp>
#include <map>
#include <string>
#include <iostream>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

namespace writerfilter
{
using namespace ::std;

class WRITERFILTER_DLLPUBLIC QNameToString
{
    typedef boost::shared_ptr<QNameToString> Pointer_t;
    typedef map < Id, string > Map;

    static Pointer_t pInstance;

    void init_doctok();
    void init_ooxml();

    Map mMap;

protected:
    /**
       Generated.
     */
    QNameToString();

public:
    static Pointer_t Instance();

    string operator()(Id qName);
};

class WRITERFILTER_DLLPUBLIC SprmIdToString
{
    typedef boost::shared_ptr<SprmIdToString> Pointer_t;

    static Pointer_t pInstance;

    map<sal_uInt32, string> mMap;

protected:
    /**
       Generated automatically.
    */
    SprmIdToString();

public:
    static Pointer_t Instance();
    string operator()(sal_uInt32 nId);
};

string WRITERFILTER_DLLPUBLIC xmlify(const string & str);

string WRITERFILTER_DLLPUBLIC propertysetToString
(uno::Reference<beans::XPropertySet> const & rProps);

}

#endif // INCLUDED_QNAME_TO_STRING_HXX
