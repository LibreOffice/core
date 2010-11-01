/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef VBAPALETTE_HXX
#define VBAPALETTE_HXX
#include <vbahelper/vbahelper.hxx>

class VbaPalette
{
        css::uno::Reference< css::container::XIndexAccess > mxPalette;
public:
        VbaPalette();
        // if no palette available e.g. because the document doesn't have a
        // palette defined then a default palette will be returned.
        css::uno::Reference< css::container::XIndexAccess > getPalette() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
