/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XImage.java,v $
 * $Revision: 1.3 $
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

package com.sun.star.lib.sandbox;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Rectangle;

import java.awt.image.ImageObserver;
import java.awt.image.ImageProducer;

public interface XImage {
    public void addWatcher(ImageObserver observer);
    public boolean isWatcher(ImageObserver observer);
    public void removeWatcher(ImageObserver observer);
    public void notifyWatchers(Rectangle refreshed);

    public int getWidth(ImageObserver observer);
    public int getHeight(ImageObserver observer);
    public Object getProperty(String name, ImageObserver observer);
    public boolean prepare(int w, int h, ImageObserver o);
    public int check(int w, int h, ImageObserver o);

    public Graphics getGraphics();

    public ImageProducer getSource();

    public void flush();

    public boolean drawOn(int xGraphicsRef, int xDeviceRef,
                          Color bgColor,
                          ImageObserver observer,
                          int dx1, int dy1, int dx2, int dy2,
                          int sx1, int sy1, int sx2, int sy2);

    public boolean drawOn(int xGraphicsRef, int xDeviceRef, int x, int y, ImageObserver observer);

    public boolean drawOn(int xGraphicsRef, int xDeviceRef, int x, int y, java.awt.Color bgColor, ImageObserver observer);
    public boolean drawOn(int xGraphicsRef, int xDeviceRef, int x, int y, int width, int height, Color bgColor, ImageObserver observer);
    public boolean drawOn(int xGraphicsRef, int xDeviceRef, int x, int y, int width, int height, ImageObserver observer);
}

