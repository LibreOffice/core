/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AudioProxy.java,v $
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

import java.applet.AudioClip;

import java.util.Vector;

class AudioProxy implements AudioClip, Holder {
    private static final boolean DEBUG = false;
    private static int instances;

    private AudioClip audioClip;
    private Vector cargo = new Vector();

    AudioProxy(AudioClip audioClip) {
        this.audioClip = audioClip;
        instances ++;
    }

    public void play() {
        audioClip.play();
    }

    public void loop() {
        audioClip.loop();
    }

    public void stop() {
        audioClip.stop();
    }

    public void addCargo(Object object) {
        cargo.addElement(object);
    }

    public void removeCargo(Object object) {
        cargo.removeElement(object);
    }

    public void finalize() {
        if(DEBUG) System.err.println("#### AudioProxy.finalize");
        instances --;
    }
}

