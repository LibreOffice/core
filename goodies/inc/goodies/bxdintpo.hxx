/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bxdintpo.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:33:08 $
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

#ifndef _BXD_INTERPOLATOR_HXX
#define _BXD_INTERPOLATOR_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#ifndef _BGFX_VECTOR_B3DVECTOR_HXX
#include <basegfx/vector/b3dvector.hxx>
#endif

/*************************************************************************
|*
|* Standard-Interpolierer
|*
\************************************************************************/

class BxdInterpolator
{
private:
    double      fPos;
    double      fInc;

public:
    BxdInterpolator(long nS, long nE, long nSteps)
    :   fPos(nSteps ? ((double)nS) + 0.5 : ((double)nE) + 0.5),
    fInc((double)(nE - nS)/(double)(nSteps == 0 ? 1 : nSteps)) {}

    BxdInterpolator(UINT8 nS, UINT8 nE, long nSteps)
    :   fPos(nSteps ? ((double)nS) + 0.5 : ((double)nE) + 0.5),
        fInc((double)(nE - nS)/(double)(nSteps == 0 ? 1 : nSteps)) {}

    BxdInterpolator(double fS, double fE, long nSteps)
    :   fPos(nSteps ? fS : fE),
        fInc((fE - fS)/(double)(nSteps == 0 ? 1 : nSteps)) {}

    BxdInterpolator() {}

    void Increment() { fPos += fInc; }

    void Load(long nS, long nE, long nSteps)
    {
        fPos = (nSteps) ? ((double)nS) + 0.5 : ((double)nE) + 0.5;
        fInc = (double)(nE - nS)/(double)(nSteps == 0 ? 1 : nSteps);
    }

    void Load(UINT8 nS, UINT8 nE, long nSteps)
    {
        fPos = (nSteps) ? ((double)nS) + 0.5 : ((double)nE) + 0.5;
        fInc = (double)(nE - nS)/(double)(nSteps == 0 ? 1 : nSteps);
    }

    void Load(double fS, double fE, long nSteps)
    {
        fPos = (nSteps) ? fS : fE;
        fInc = (fE - fS)/(double)(nSteps == 0 ? 1 : nSteps);
    }

    long GetLongValue() { return (long)fPos; }
    UINT32 GetUINT32Value() { return (UINT32)fPos; }
    UINT8 GetUINT8Value() { return (UINT8)fPos; }
    double GetDoubleValue() { return fPos; }
    double GetStepSize() { return fInc; }
};

/*************************************************************************
|*
|* Interpolierer fuer Color
|*
\************************************************************************/

class BxdColorInterpolator
{
private:
    BxdInterpolator         aRed;
    BxdInterpolator         aGreen;
    BxdInterpolator         aBlue;
    UINT8                   nTransparency;

public:
    BxdColorInterpolator(Color aS, Color aE, long nSteps)
    :   aRed(aS.GetRed(), aE.GetRed(), nSteps),
        aGreen(aS.GetGreen(), aE.GetGreen(), nSteps),
        aBlue(aS.GetBlue(), aE.GetBlue(), nSteps),
        nTransparency(aS.GetTransparency()) {}

    BxdColorInterpolator() {}

    void Increment()
    {
        aRed.Increment();
        aGreen.Increment();
        aBlue.Increment();
    }

    void Load(Color aS, Color aE, long nSteps)
    {
        aRed.Load(aS.GetRed(), aE.GetRed(), nSteps);
        aGreen.Load(aS.GetGreen(), aE.GetGreen(), nSteps);
        aBlue.Load(aS.GetBlue(), aE.GetBlue(), nSteps);
        nTransparency = aS.GetTransparency();
    }

    Color GetColorValue()
    {
        return Color(nTransparency,
            aRed.GetUINT8Value(),
            aGreen.GetUINT8Value(),
            aBlue.GetUINT8Value());
    }
};

/*************************************************************************
|*
|* Interpolierer fuer Vector3D
|*
\************************************************************************/

class B3dVectorInterpolator
{
private:
    BxdInterpolator         aX;
    BxdInterpolator         aY;
    BxdInterpolator         aZ;

public:
    B3dVectorInterpolator(basegfx::B3DVector& rS, basegfx::B3DVector& rE, long nSteps)
    :   aX(rS.getX(), rE.getX(), nSteps),
        aY(rS.getY(), rE.getY(), nSteps),
        aZ(rS.getZ(), rE.getZ(), nSteps) {}

    B3dVectorInterpolator() {}

    void Increment() { aX.Increment(); aY.Increment(); aZ.Increment(); }

    void Load(basegfx::B3DVector& rS, basegfx::B3DVector& rE, long nSteps)
    {
        aX.Load(rS.getX(), rE.getX(), nSteps);
        aY.Load(rS.getY(), rE.getY(), nSteps);
        aZ.Load(rS.getZ(), rE.getZ(), nSteps);
    }

    void GetVector3DValue(basegfx::B3DVector& rVec)
    {
        rVec.setX(aX.GetDoubleValue());
        rVec.setY(aY.GetDoubleValue());
        rVec.setZ(aZ.GetDoubleValue());
    }
};

#endif          // _BXD_INTERPOLATOR_HXX
