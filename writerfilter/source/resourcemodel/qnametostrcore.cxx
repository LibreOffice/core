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

#include <resourcemodel/QNameToString.hxx>
#include <doctok/WW8Document.hxx>
#include <ooxml/OOXMLDocument.hxx>

namespace writerfilter
{

QNameToString::Pointer_t QNameToString::pInstance;

QNameToString::Pointer_t WRITERFILTER_RESOURCEMODEL_DLLPUBLIC QNameToString::Instance()
{
    if (pInstance.get() == NULL)
        pInstance = QNameToString::Pointer_t(new QNameToString());

    return pInstance;
}

string WRITERFILTER_RESOURCEMODEL_DLLPUBLIC QNameToString::operator()(Id qName)
{
    string sResult;

    Map::const_iterator aIt = mMap.find(qName);
    
    if (aIt != mMap.end())
        sResult = aIt->second;

    return mMap[qName];
}

QNameToString::QNameToString()
{
    init_doctok();
    init_ooxml();
}

extern void ooxmlsprmidsToXML(::std::ostream &out);
extern void sprmidsToXML(::std::ostream &out);
extern void doctokidsToXML(::std::ostream &out);
extern void ooxmlidsToXML(::std::ostream &out);

void WRITERFILTER_RESOURCEMODEL_DLLPUBLIC analyzerIds()
{
    cout << "<ids type=\"sprm\">" << endl;

    sprmidsToXML(cout);
    ooxmlsprmidsToXML(cout);

    cout << "</ids>" << endl;

    cout << "<ids type=\"attribute\">" << endl;

    doctokidsToXML(cout);
    ooxmlidsToXML(cout);

    cout << "</ids>" << endl;
}

}
