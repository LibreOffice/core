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
#ifndef _SCH_TRANSFERABLE_HXX_
#define _SCH_TRANSFERABLE_HXX_

#include <bf_svtools/transfer.hxx>

/*N*/ #include <tools/debug.hxx> //for stripping
class Graphic;
class VirtualDevice;
namespace binfilter {
class SdrModel;
class SdrExchangeView;

class SchTransferable :
    public TransferableHelper
{
private:
    SdrExchangeView  *              mpSourceView;

protected:

    // implementation of TransferableHelper methods

public:

    /** CTOR for clipboard, drag and drop and selection clipboard
        @param pObjModel the model containing all objects for the clipboard.
                         The transferable becomes the owner of this model.
        @param pSrcView  the view for drag and drop and the selection clipboard
                         The transferable does not care about destruction of this view
        @param rObjDesc  A descriptor object that holds properties like the starting
                         position of a drag action
        @param bLateInit If true, the actual data is created in GetData (on paste),
                         else data is created on construction
     */
    SchTransferable( SdrModel* pObjModel,
                     SdrExchangeView* pSrcView,
                     const TransferableObjectDescriptor& rObjDesc,
                     sal_Bool bLateInit = sal_False );
      const SdrExchangeView* GetView() throw()		{ return mpSourceView; }
};

} //namespace binfilter
#endif	// _SCH_TRANSFERABLE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
