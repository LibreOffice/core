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
#ifndef INCLUDED_SVX_SOURCE_GALLERY2_GALLERYSPLITTER_HXX
#define INCLUDED_SVX_SOURCE_GALLERY2_GALLERYSPLITTER_HXX

#include <vcl/split.hxx>
#include <boost/function.hpp>

class GallerySplitter : public Splitter
{
public:
    GallerySplitter (
        Window* pParent,
        const ResId& rResId,
        const ::boost::function<void(void)>& rDataChangeFunctor);
    virtual ~GallerySplitter (void);

protected:
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

private:
    ::boost::function<void(void)> maDataChangeFunctor;
};


#endif
