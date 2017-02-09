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
#ifndef INCLUDED_RSC_INC_RSCCLOBJ_HXX
#define INCLUDED_RSC_INC_RSCCLOBJ_HXX

#include <rsctree.hxx>
#include <rscdef.hxx>
#include <rscall.h>

/******************* O b j N o d e ***************************************/
class ObjNode : public IdNode
{
    RscId       aRscId; // Id der Resource
    CLASS_DATA  pRscObj;// pointer to a resourceobject
    RscFileTab::Index lFileKey;// Dateischluessel
protected:
    using NameNode::Search;

public:
    using NameNode::Insert;

                ObjNode( const RscId & rId, CLASS_DATA pData, RscFileTab::Index lKey );
    ObjNode *   DelObjNode( RscTop * pClass, RscFileTab::Index lFileKey );
    sal_uInt32  GetId() const override;
    const RscId& GetRscId() const { return aRscId; }
    RscFileTab::Index GetFileKey() const { return lFileKey; };
    ObjNode*    Search( const RscId &rName ) const //< search the index in the b-tree
                    {
                        return static_cast<ObjNode *>(IdNode::Search( rName.GetNumber() ));
                    }
    bool        Insert( ObjNode* pTN ) //< insert a new node in the b-tree

                    {
                        return IdNode::Insert( static_cast<IdNode *>(pTN) );
                    }
    CLASS_DATA  GetRscObj() const//< get the Object from this Node

                    {
                        return pRscObj;
                    }
    bool        IsConsistent();
};

/******************* R e f N o d e ***************************************/
class RefNode : public IdNode
{
    Atom        nTypNameId; // index of a Name in a hashtabel
protected:
    using NameNode::Search;

public:
    using NameNode::Insert;

    ObjNode*    pObjBiTree; // Zeiger auf Objektbaum
                RefNode( Atom nTyp );
    sal_uInt32  GetId() const override;
    void        Insert( RefNode* pTN ) //< insert a new node in the b-tree
                    {
                        IdNode::Insert( static_cast<IdNode *>(pTN) );
                    }
    bool        PutObjNode( ObjNode * pPutObject );

                // insert new node in b-tree pObjBiTree
    ObjNode *   GetObjNode( const RscId &rRscId );

    ObjNode *   GetObjNode() const
                    {
                        // hole  pObjBiTree
                        return pObjBiTree;
                    }
};

#endif // INCLUDED_RSC_INC_RSCCLOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
