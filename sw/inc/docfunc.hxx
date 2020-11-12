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
#ifndef INCLUDED_SW_INC_DOCFUNC_HXX
#define INCLUDED_SW_INC_DOCFUNC_HXX

class SwDoc;

// namespace <docfunc> for functions and procedures working on a Writer document.
namespace docfunc
{
/** method to check, if given Writer document contains at least one drawing object

    @param p_rDoc
    input parameter - reference to the Writer document, which is investigated.
*/
bool ExistsDrawObjs(SwDoc& p_rDoc);

/** method to check, if given Writer document contains only drawing objects,
    which are completely on its page.

    @param p_rDoc
    input parameter - reference to the Writer document, which is investigated.
*/
bool AllDrawObjsOnPage(SwDoc& p_rDoc);

/** method to check, if the outline style has to written as a normal list style

    #i69627#
    The outline style has to written as a normal list style, if a parent
    paragraph style of one of the paragraph styles, which are assigned to
    the list levels of the outline style, has a list style set or inherits
    a list style from its parent paragraphs style.
    This information is needed for the OpenDocument file format export.

    @param rDoc
    input parameter - reference to the text document, which is investigated.

    @return boolean
    indicating, if the outline style has to written as a normal list style
*/
bool HasOutlineStyleToBeWrittenAsNormalListStyle(SwDoc& rDoc);
}

#endif //_DOCFUNC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
