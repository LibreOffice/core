/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "sal/config.h"

#include "boost/noncopyable.hpp"
#include "boost/unordered_map.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/container/XNameContainer.hpp"
#include "com/sun/star/drawing/ColorTable.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "comphelper/processfactory.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "vcl/svapp.hxx"

namespace {

class ColorNameMap: private boost::noncopyable {
public:
    ColorNameMap();

    OUString lookUp(long color) const;

private:
    typedef boost::unordered_map< long, OUString > Map;

    Map map_;
};

ColorNameMap::ColorNameMap() {
    css::uno::Sequence< OUString > aNames;
    css::uno::Reference< css::container::XNameAccess > xNA;

    try
    {
        
            css::uno::Reference< css::container::XNameContainer > xColorTable =
                 css::drawing::ColorTable::create( comphelper::getProcessComponentContext() );

        

        
        
        SolarMutexGuard aGuard;
        aNames = xNA->getElementNames();
    }
    catch (css::uno::RuntimeException const&)
    {
        
        
    }

    
    if (xNA.is())
        for (long int i=0; i<aNames.getLength(); i++)
        {
            
            try
            {
                css::uno::Any aColor (xNA->getByName (aNames[i]));
                long nColor = 0;
                aColor >>= nColor;
                map_[nColor] = aNames[i];
            }
            catch (css::uno::RuntimeException const&)
            {
                
                
            }
        }
}

OUString ColorNameMap::lookUp(long color) const {
    Map::const_iterator i(map_.find(color));
    if (i != map_.end()) {
        return i->second;
    }
    
    OUStringBuffer buf;
    buf.append('#');
    buf.append(color, 16);
    return buf.makeStringAndClear();
}

struct theColorNameMap: public rtl::Static< ColorNameMap, theColorNameMap > {};

}

namespace accessibility {

OUString lookUpColorName(long color) {
    return theColorNameMap::get().lookUp(color);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
