/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef INCLUDED_SVX_SVDOBJUSERDATALIST_HXX
#define INCLUDED_SVX_SVDOBJUSERDATALIST_HXX

#include <svx/svdobj.hxx>

#include <boost/ptr_container/ptr_vector.hpp>

class SdrObjUserData;

class SdrObjUserDataList
{
    typedef boost::ptr_vector<SdrObjUserData> ListType;
    ListType maList;

public:
    SdrObjUserDataList();
    ~SdrObjUserDataList();

    size_t GetUserDataCount() const;
    SdrObjUserData& GetUserData(size_t nNum);
    void AppendUserData(SdrObjUserData* pData);
    void DeleteUserData(size_t nNum);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
