/***************************************************************************
    iIterator.hxx  -  iIterator - header file
    iterates over all OLE objects (formulas or charts) in a document
                             -------------------
    begin                : Wed Mar 4 2020
    copyright            : (C) 2020 by Jan Rheinlaender
    email                : jrheinlaender@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _IITERATOR_HXX
#define _IITERATOR_HXX

#include <string>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/frame/XModel.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/container/XIndexAccess.hpp"
#include "com/sun/star/drawing/XShapes.hpp"

using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using rtl::OUString;
using com::sun::star::lang::XComponent;
using com::sun::star::frame::XModel;
using com::sun::star::container::XNameAccess;
using com::sun::star::container::XIndexAccess;
using com::sun::star::drawing::XShapes;

class iIterator {
public:
  iIterator(const Reference< XModel >& xModel);
  iIterator(const Reference< XModel >& xModel, const OUString& classid);

  bool next();
  size_t currentCount();
  size_t count();
  Reference< XComponent > operator*();

  // If iterating on a Presentation, return the XShapes collection in which the current object is located
  Reference< XShapes > getCurrentShapes();

private:
  std::string type;
  Reference< XModel > model;
  OUString clsid;
  int numObjects;
  int numCurrentObject;

  // For TextDocument
  Reference< XNameAccess > embeddedObjects;
  Sequence< OUString > elementNames;
  // For Presentation
  Reference< XIndexAccess > xDrawPages;
  int currentDrawPage;
  int numDrawPages;
  int currentShape;
  int numShapes;
};

#endif
