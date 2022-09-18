 /***************************************************************************
    iIterator.hxx  -  iIterator - Implementation
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

#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/document/XEmbeddedObjectSupplier.hpp"
#include "com/sun/star/drawing/XDrawPagesSupplier.hpp"
#include "com/sun/star/drawing/XDrawPages.hpp"
#include "com/sun/star/drawing/XDrawPage.hpp"

#include "iIterator.hxx"
#ifdef INSIDE_SM
#include <imath/imathutils.hxx>
#include <imath/msgdriver.hxx>
#else
#include "imathutils.hxx"
#include "msgdriver.hxx"
#endif

using com::sun::star::lang::XServiceInfo;
using com::sun::star::document::XEmbeddedObjectSupplier;
using com::sun::star::drawing::XDrawPagesSupplier;
using com::sun::star::drawing::XDrawPages;
using com::sun::star::drawing::XDrawPage;

iIterator::iIterator(const Reference< XModel >& xModel) : iIterator(xModel, CLSID_FORMULA) {};

iIterator::iIterator(const Reference< XModel >& xModel, const OUString& classid) : model(xModel), clsid(classid) {
  type = STR(docType(xModel)) ;
  MSG_INFO(0, "iIterator for " << type << endline);

  if (type == "TextDocument") {
    // TODO: Is there a way to iterate through the embedded objects directly (XNameAccess) ?
    Reference< XTextEmbeddedObjectsSupplier > xTEOS(xModel, UNO_QUERY_THROW);
    embeddedObjects = xTEOS->getEmbeddedObjects();
    elementNames = embeddedObjects->getElementNames();
    numObjects = elementNames.getLength();
  } else if (type == "Presentation") {
    Reference< XDrawPagesSupplier > xPresDoc(xModel, UNO_QUERY_THROW);
    xDrawPages = Reference< XIndexAccess > (xPresDoc->getDrawPages(), UNO_QUERY_THROW);
    numDrawPages = xDrawPages->getCount();
    currentDrawPage = 0;
    Reference< XDrawPage > xPage(xDrawPages->getByIndex(currentDrawPage), UNO_QUERY_THROW);
    Reference< XIndexAccess > xShapes(xPage, UNO_QUERY_THROW);
    numShapes = xShapes->getCount();
    currentShape = -1;
    numObjects = 0;

    // Count total number of objects
    for (int idx = 0; idx < numDrawPages; ++idx) {
      Reference< XDrawPage > page(xDrawPages->getByIndex(idx), UNO_QUERY_THROW);
      Reference< XIndexAccess > shapes(page, UNO_QUERY_THROW);

      for (int sh = 0; sh < shapes->getCount(); ++sh) {
        Reference< XServiceInfo > xSI(shapes->getByIndex(sh), UNO_QUERY_THROW);

        if (xSI.is() && xSI->supportsService(OU("com.sun.star.drawing.OLE2Shape"))) {
          Reference< XComponent > comp(shapes->getByIndex(sh), UNO_QUERY);

          if (comp.is() && checkIsObject(comp, clsid))
            ++numObjects;
        }
      }
    }
  }

  MSG_INFO(0, "Initialized for " << numObjects << " objects" << endline);
  numCurrentObject = -1; // So that loops can be written as: while (it.next()) {}
}

bool iIterator::next() {
  MSG_INFO(0, "iIterator::next() for " << type << ", currentObject=" << numCurrentObject << endline);
  ++numCurrentObject;

  if (type == "TextDocument") {
    return (numCurrentObject < numObjects);
  } else if (type == "Presentation") {
    do {
      MSG_INFO(0, "Page " << (currentDrawPage+1) << " of " << numDrawPages << ", Shape " << (currentShape+1) << " of " << numShapes << endline);
      ++currentShape;

      // Find next shape
      Reference< XDrawPage > xPage(xDrawPages->getByIndex(currentDrawPage), UNO_QUERY_THROW);
      Reference< XIndexAccess > xShapes(xPage, UNO_QUERY);

      while (currentShape == numShapes && currentDrawPage < numDrawPages) {
        ++currentDrawPage;
        if (currentDrawPage == numDrawPages) return false; // No more shapes in the Presentation
        xPage = Reference< XDrawPage >(xDrawPages->getByIndex(currentDrawPage), UNO_QUERY_THROW);
        xShapes = Reference< XIndexAccess >(xPage, UNO_QUERY_THROW);
        numShapes = xShapes->getCount();
        currentShape = 0;
      }

      // Check if shape is matching object
      Reference< XServiceInfo > xSI(xShapes->getByIndex(currentShape), UNO_QUERY_THROW);
      if (xSI.is() && xSI->supportsService(OU("com.sun.star.drawing.OLE2Shape"))) {
        Reference< XComponent > comp(xShapes->getByIndex(currentShape), UNO_QUERY_THROW);

        if (checkIsObject(comp, clsid))
          return true;
      }
    } while (currentDrawPage < numDrawPages);

    return false;
  } else {
    return false;
  }
}

size_t iIterator::currentCount() {
  return numCurrentObject;
}

size_t iIterator::count() {
  return numObjects;
}

Reference< XComponent > iIterator::operator*() {
  if (type == "TextDocument")
    return getObjectByName(embeddedObjects, elementNames[numCurrentObject]);
  else if (type == "Presentation") {
    MSG_INFO(0, "Accessing page " << (currentDrawPage+1) << ", shape " << (currentShape+1) << endline);
    Reference< XDrawPage > xPage(xDrawPages->getByIndex(currentDrawPage), UNO_QUERY_THROW);
    Reference< XIndexAccess > xShapes(xPage, UNO_QUERY_THROW);
    return Reference< XComponent >(xShapes->getByIndex(currentShape), UNO_QUERY_THROW);
  } else {
    return Reference< XComponent >();
  }
}

Reference< XShapes > iIterator::getCurrentShapes() {
  if (type == "Presentation") {
    Reference< XDrawPage > xPage(xDrawPages->getByIndex(currentDrawPage), UNO_QUERY_THROW);
    return Reference< XShapes >(xPage, UNO_QUERY_THROW);
  } else {
    return Reference< XShapes >();
  }
}
