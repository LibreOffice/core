/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XImage.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:10:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

