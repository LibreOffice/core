/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Lubos Lunak <l.lunak@suse.cz> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef SM_OOXML_HXX
#define SM_OOXML_HXX

#include "node.hxx"

#include <sax/fshelper.hxx>

/**
 Class implementing writing of formulas to OOXML.
 */
class SmOoxml
{
public:
    SmOoxml(String &rIn,SmNode *pIn);
    bool ConvertFromStarMath( ::sax_fastparser::FSHelperPtr m_pSerializer );
private:
    void HandleNodes(SmNode *pNode,int nLevel);
    void HandleTable(SmNode *pNode,int nLevel);
    void HandleText(SmNode *pNode,int nLevel);
    void HandleMath(SmNode *pNode,int nLevel);
    String str;
    SmNode *pTree;
    ::sax_fastparser::FSHelperPtr m_pSerializer;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
