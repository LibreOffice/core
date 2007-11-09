/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OGLTrans_TransitionImpl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-09 10:17:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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
#include "OGLTrans_TransitionImpl.hxx"
#include <GL/gl.h>

void OGLTransitionImpl::clear()
{
    for(unsigned int i( 0 ); i < OverallOperations.size(); ++i)
        delete OverallOperations[i];
    OverallOperations.clear();
    maLeavingSlidePrimitives.clear();
    maEnteringSlidePrimitives.clear();
}

OGLTransitionImpl::~OGLTransitionImpl()
{
    clear();
}

void OGLTransitionImpl::display( double nTime, ::sal_Int32 glLeavingSlideTex, ::sal_Int32 glEnteringSlideTex , double SlideWidthScale, double SlideHeightScale)
{
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    //TODO change to foreach
    for(unsigned int i(0); i < OverallOperations.size(); ++i)
        OverallOperations[i]->interpolate(nTime,SlideWidthScale,SlideHeightScale);
    glBindTexture(GL_TEXTURE_2D, glLeavingSlideTex);
    for(unsigned int i(0); i < maLeavingSlidePrimitives.size(); ++i)
        maLeavingSlidePrimitives[i].display(nTime,SlideWidthScale,SlideHeightScale);
    glBindTexture(GL_TEXTURE_2D, glEnteringSlideTex);
    for(unsigned int i(0); i < maEnteringSlidePrimitives.size(); ++i)
        maEnteringSlidePrimitives[i].display(nTime,SlideWidthScale,SlideHeightScale);
    glPopMatrix();
}

void Primitive::display(double nTime, double SlideWidthScale, double SlideHeightScale)
{
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    for(unsigned int i(0); i < Operations.size(); ++i)
        Operations[i]->interpolate( nTime ,SlideWidthScale,SlideHeightScale);
    glScaled(SlideWidthScale,SlideHeightScale,1);

    glEnableClientState( GL_VERTEX_ARRAY );
    if(!Normals.empty())
    {
        glNormalPointer( GL_DOUBLE , 0 , &Normals[0] );
        glEnableClientState( GL_NORMAL_ARRAY );
    }
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    glTexCoordPointer( 2 , GL_DOUBLE , 0 , &TexCoords[0] );
    glVertexPointer( 3 , GL_DOUBLE , 0 , &Vertices[0] );
    glDrawArrays( GL_TRIANGLES , 0 , Vertices.size() );
    glPopMatrix();
}

Primitive::~Primitive()
{
    for(unsigned int i( 0 ); i < Operations.size(); ++i)
        delete Operations[i];
}

void OGLTransitionImpl::makeOutsideCubeFaceToLeft()
{
    clear();
    Primitive Slide;

    Slide.pushTriangle(basegfx::B2DVector(0,0),basegfx::B2DVector(1,0),basegfx::B2DVector(0,1));
    Slide.pushTriangle(basegfx::B2DVector(1,0),basegfx::B2DVector(0,1),basegfx::B2DVector(1,1));

    maLeavingSlidePrimitives.push_back(Slide);

    Slide.Operations.push_back(new RotateAndScaleDepthByWidth(basegfx::B3DVector(0,1,0),basegfx::B3DVector(0,0,-1),-90,false,0.0,1.0));

    maEnteringSlidePrimitives.push_back(Slide);

    OverallOperations.push_back(new RotateAndScaleDepthByWidth(basegfx::B3DVector(0,1,0),basegfx::B3DVector(0,0,-1),90,true,0.0,1.0));
}

void OGLTransitionImpl::makeInsideCubeFaceToLeft()
{
    clear();
    Primitive Slide;

    Slide.pushTriangle(basegfx::B2DVector(0,0),basegfx::B2DVector(1,0),basegfx::B2DVector(0,1));
    Slide.pushTriangle(basegfx::B2DVector(1,0),basegfx::B2DVector(0,1),basegfx::B2DVector(1,1));

    maLeavingSlidePrimitives.push_back(Slide);

    Slide.Operations.push_back(new RotateAndScaleDepthByWidth(basegfx::B3DVector(0,1,0),basegfx::B3DVector(0,0,1),90,false,0.0,1.0));

    maEnteringSlidePrimitives.push_back(Slide);

    OverallOperations.push_back(new RotateAndScaleDepthByWidth(basegfx::B3DVector(0,1,0),basegfx::B3DVector(0,0,1),-90,true,0.0,1.0));
}

// TODO(Q3): extract to basegfx
inline basegfx::B2DVector clamp(const basegfx::B2DVector& v)
{
    return basegfx::B2DVector(std::min(std::max(v.getX(),-1.0),1.0),
                              std::min(std::max(v.getY(),-1.0),1.0));
}

// TODO(Q3): extract to basegfx
inline basegfx::B3DVector clamp(const basegfx::B3DVector& v)
{
    return basegfx::B3DVector(std::min(std::max(v.getX(),-1.0),1.0),
                              std::min(std::max(v.getY(),-1.0),1.0),
                              std::min(std::max(v.getZ(),-1.0),1.0));
}

inline double randFromNeg1to1()
{
    return ( ( static_cast<double>( rand() ) / static_cast<double>( RAND_MAX ) ) * 2.0 ) - 1.0;
}

// TODO(Q3): extract to basegfx
inline basegfx::B3DVector randNormVectorInXYPlane()
{
    basegfx::B3DVector toReturn(randFromNeg1to1(),randFromNeg1to1(),0.0);
    return toReturn/toReturn.getLength();
}

void OGLTransitionImpl::makeRevolvingCircles( ::sal_uInt16 nCircles , ::sal_uInt16 nPointsOnCircles )
{
    clear();
    double dAngle(2*3.1415926/static_cast<double>( nPointsOnCircles ));
    if(nCircles < 2 || nPointsOnCircles < 4)
    {
        makeNByMTileFlip(1,1);
        return;
    }
    double Radius(1.0/static_cast<double>( nCircles ));
    double dRadius(Radius);
    double LastRadius(0.0);
    double NextRadius(2*Radius);

    /// now we know there is at least two circles
    /// the first will always be a full circle
    /// the last will always be the outer shell of the slide with a circle hole

    //add the full circle
    std::vector<basegfx::B2DVector> unScaledTexCoords;
    double TempAngle(0.0);
    for(unsigned int Point(0); Point < nPointsOnCircles; ++Point)
    {
        unScaledTexCoords.push_back( basegfx::B2DVector( cos(TempAngle - 3.1415926/2.0) , sin(TempAngle- 3.1415926/2.0) ) );

        TempAngle += dAngle;
    }

    {
        //double angle(0.0);
        Primitive EnteringSlide;
        Primitive LeavingSlide;
        for(int Point(0); Point + 1 < nPointsOnCircles; ++Point)
        {
            EnteringSlide.pushTriangle( basegfx::B2DVector( 0.5 , 0.5 ) , Radius * unScaledTexCoords[ Point + 1 ] / 2.0 + basegfx::B2DVector( 0.5 , 0.5 ) , Radius * unScaledTexCoords[ Point ] / 2.0 + basegfx::B2DVector( 0.5 , 0.5 ) );
            LeavingSlide.pushTriangle( basegfx::B2DVector( 0.5 , 0.5 ) , Radius * unScaledTexCoords[ Point + 1 ] / 2.0 + basegfx::B2DVector( 0.5 , 0.5 ) , Radius * unScaledTexCoords[ Point ] / 2.0 + basegfx::B2DVector( 0.5, 0.5) );
        }
        EnteringSlide.pushTriangle( basegfx::B2DVector(0.5,0.5) , Radius * unScaledTexCoords[ 0 ] / 2.0 + basegfx::B2DVector( 0.5 , 0.5 ) , Radius * unScaledTexCoords[ nPointsOnCircles - 1 ] / 2.0 + basegfx::B2DVector( 0.5 , 0.5 ) );
        LeavingSlide.pushTriangle( basegfx::B2DVector(0.5,0.5) , Radius*unScaledTexCoords[0]/2.0 + basegfx::B2DVector(0.5,0.5) , Radius*unScaledTexCoords[nPointsOnCircles - 1]/2.0 + basegfx::B2DVector(0.5,0.5) );

        basegfx::B3DVector axis(randNormVectorInXYPlane());
        EnteringSlide.Operations.push_back( new SRotate( axis , basegfx::B3DVector(0,0,0) , 180, true, Radius/2.0 , (NextRadius + 1)/2.0 ) );
        LeavingSlide.Operations.push_back( new SRotate( axis , basegfx::B3DVector(0,0,0) , 180, true, Radius/2.0 , (NextRadius + 1)/2.0 ) );
        EnteringSlide.Operations.push_back( new SRotate( axis , basegfx::B3DVector(0,0,0) , -180, false,0.0,1.0) );

        maEnteringSlidePrimitives.push_back(EnteringSlide);
        maLeavingSlidePrimitives.push_back(LeavingSlide);
        LastRadius = Radius;
        Radius = NextRadius;
        NextRadius += dRadius;
    }

    for(int i(1); i < nCircles - 1; ++i)
    {
        Primitive LeavingSlide;
        Primitive EnteringSlide;
        for(int Side(0); Side < nPointsOnCircles - 1; ++Side)
        {
            EnteringSlide.pushTriangle(Radius*unScaledTexCoords[Side]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0 + basegfx::B2DVector(0.5,0.5) );
            EnteringSlide.pushTriangle(Radius*unScaledTexCoords[Side]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0 + basegfx::B2DVector(0.5,0.5) , Radius*unScaledTexCoords[Side + 1]/2.0 + basegfx::B2DVector(0.5,0.5) );

            LeavingSlide.pushTriangle(Radius*unScaledTexCoords[Side]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0 + basegfx::B2DVector(0.5,0.5) );
            LeavingSlide.pushTriangle(Radius*unScaledTexCoords[Side]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0 + basegfx::B2DVector(0.5,0.5) , Radius*unScaledTexCoords[Side + 1]/2.0 + basegfx::B2DVector(0.5,0.5) );
        }

        EnteringSlide.pushTriangle(Radius*unScaledTexCoords[nPointsOnCircles - 1]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[nPointsOnCircles - 1]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0 + basegfx::B2DVector(0.5,0.5) );
        EnteringSlide.pushTriangle(Radius*unScaledTexCoords[nPointsOnCircles - 1]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0 + basegfx::B2DVector(0.5,0.5) , Radius*unScaledTexCoords[0]/2.0 + basegfx::B2DVector(0.5,0.5) );

        LeavingSlide.pushTriangle(Radius*unScaledTexCoords[nPointsOnCircles - 1]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[nPointsOnCircles - 1]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0 + basegfx::B2DVector(0.5,0.5) );
        LeavingSlide.pushTriangle(Radius*unScaledTexCoords[nPointsOnCircles - 1]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0 + basegfx::B2DVector(0.5,0.5) , Radius*unScaledTexCoords[0]/2.0 + basegfx::B2DVector(0.5,0.5) );

        basegfx::B3DVector axis(randNormVectorInXYPlane());
        EnteringSlide.Operations.push_back( new SRotate( axis , basegfx::B3DVector(0,0,0) , 180, true, Radius/2.0 , (NextRadius + 1)/2.0 ) );
        LeavingSlide.Operations.push_back( new SRotate( axis , basegfx::B3DVector(0,0,0) , 180, true, Radius/2.0 , (NextRadius + 1)/2.0 ) );
        EnteringSlide.Operations.push_back( new SRotate( axis , basegfx::B3DVector(0,0,0) , -180, false,0.0,1.0) );

        maEnteringSlidePrimitives.push_back(EnteringSlide);
        maLeavingSlidePrimitives.push_back(LeavingSlide);

        LastRadius = Radius;
        Radius = NextRadius;
        NextRadius += dRadius;
    }
    {
        Radius = sqrt(2);
        Primitive LeavingSlide;
        Primitive EnteringSlide;
        for(int Side(0); Side < nPointsOnCircles - 1; ++Side)
        {

            EnteringSlide.pushTriangle(clamp(Radius*unScaledTexCoords[Side])/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0 + basegfx::B2DVector(0.5,0.5) );
            EnteringSlide.pushTriangle(clamp(Radius*unScaledTexCoords[Side])/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0 + basegfx::B2DVector(0.5,0.5) , clamp(Radius*unScaledTexCoords[Side + 1])/2.0 + basegfx::B2DVector(0.5,0.5) );

            LeavingSlide.pushTriangle(clamp(Radius*unScaledTexCoords[Side])/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0 + basegfx::B2DVector(0.5,0.5) );
            LeavingSlide.pushTriangle(clamp(Radius*unScaledTexCoords[Side])/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[Side + 1]/2.0 + basegfx::B2DVector(0.5,0.5) , clamp(Radius*unScaledTexCoords[Side + 1])/2.0 + basegfx::B2DVector(0.5,0.5) );
        }

        EnteringSlide.pushTriangle(clamp(Radius*unScaledTexCoords[nPointsOnCircles - 1])/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[nPointsOnCircles - 1]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0 + basegfx::B2DVector(0.5,0.5) );
        EnteringSlide.pushTriangle(clamp(Radius*unScaledTexCoords[nPointsOnCircles - 1])/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0 + basegfx::B2DVector(0.5,0.5) , clamp(Radius*unScaledTexCoords[0])/2.0 + basegfx::B2DVector(0.5,0.5) );

        LeavingSlide.pushTriangle(clamp(Radius*unScaledTexCoords[nPointsOnCircles - 1])/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[nPointsOnCircles - 1]/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0 + basegfx::B2DVector(0.5,0.5) );
        LeavingSlide.pushTriangle(clamp(Radius*unScaledTexCoords[nPointsOnCircles - 1])/2.0 + basegfx::B2DVector(0.5,0.5) , LastRadius*unScaledTexCoords[0]/2.0 + basegfx::B2DVector(0.5,0.5) , clamp(Radius*unScaledTexCoords[0])/2.0 + basegfx::B2DVector(0.5,0.5) );

        basegfx::B3DVector axis(randNormVectorInXYPlane());
        EnteringSlide.Operations.push_back( new SRotate( axis , basegfx::B3DVector(0,0,0) , 180, true, (LastRadius + dRadius)/2.0 , 1.0 ) );
        LeavingSlide.Operations.push_back( new SRotate( axis , basegfx::B3DVector(0,0,0) , 180, true, (LastRadius + dRadius)/2.0 , 1.0 ) );
        EnteringSlide.Operations.push_back( new SRotate( axis , basegfx::B3DVector(0,0,0) , -180, false,0.0,1.0) );

        maEnteringSlidePrimitives.push_back(EnteringSlide);
        maLeavingSlidePrimitives.push_back(LeavingSlide);
    }
}

void OGLTransitionImpl::makeHelix( ::sal_uInt16 nRows )
{
    clear();
    double invN(1.0/static_cast<double>(nRows));
    double iDn = 0.0;
    double iPDn = invN;
    for(unsigned int i(0); i < nRows; ++i)
    {
        Primitive Tile;

        Tile.pushTriangle(basegfx::B2DVector( 1.0 , iDn ) , basegfx::B2DVector( 0.0 , iDn ) , basegfx::B2DVector( 0.0 , iPDn ));

        Tile.pushTriangle(basegfx::B2DVector( 1.0 , iPDn ) , basegfx::B2DVector( 1.0 , iDn ) , basegfx::B2DVector( 0.0 , iPDn ));

        Tile.Operations.push_back( new SRotate( basegfx::B3DVector( 0 , 1 , 0 ) , ( Tile.getVertices()[1] + Tile.getVertices()[3] )/2.0 , 180 ,
                                                true,std::min(std::max(static_cast<double>(i - nRows/2.0)*invN/2.0,0.0),1.0),
                                                std::min(std::max(static_cast<double>(i + nRows/2.0)*invN/2.0,0.0),1.0) ) );

        maLeavingSlidePrimitives.push_back(Tile);

        Tile.Operations.push_back( new SRotate( basegfx::B3DVector( 0 , 1 , 0 ) , ( Tile.getVertices()[1] + Tile.getVertices()[3] )/2.0 , -180 , false,0.0,1.0) );

        maEnteringSlidePrimitives.push_back(Tile);

        iDn += invN;
        iPDn += invN;
    }
}

void OGLTransitionImpl::makeNByMTileFlip( ::sal_uInt16 n, ::sal_uInt16 m )
{
    clear();
    double invN(1.0/static_cast<double>(n));
    double invM(1.0/static_cast<double>(m));
    double iDn = 0.0;
    double iPDn = invN;
    for(unsigned int i(0); i < n; ++i)
    {
        double jDm = 0.0;
        double jPDm = invM;
        for(unsigned int j(0); j < m; ++j)
        {
            Primitive Tile;

            Tile.pushTriangle(basegfx::B2DVector( iPDn , jDm ) , basegfx::B2DVector( iDn , jDm ) , basegfx::B2DVector( iDn , jPDm ));

            Tile.pushTriangle(basegfx::B2DVector( iPDn , jPDm ) , basegfx::B2DVector( iPDn , jDm ) , basegfx::B2DVector( iDn , jPDm ));//bottom left corner of tile

            Tile.Operations.push_back( new SRotate( basegfx::B3DVector( 1 , 1 , 0 ) , ( Tile.getVertices()[1] + Tile.getVertices()[3] )/2.0 , 180 , true, iDn*jDm/2.0 , ((iPDn*jPDm)+1.0)/2.0 ) );
            maLeavingSlidePrimitives.push_back(Tile);
            Tile.Operations.push_back( new SRotate( basegfx::B3DVector( 1 , 1 , 0 ) , ( Tile.getVertices()[1] + Tile.getVertices()[3] )/2.0 , -180, false, iDn*jDm/2.0 , ((iPDn*jPDm)+1.0)/2.0 ) );

            maEnteringSlidePrimitives.push_back(Tile);

            jDm += invM;
            jPDm += invM;
        }
        iDn += invN;
        iPDn += invN;
    }
}

SRotate::SRotate(const basegfx::B3DVector& Axis,const basegfx::B3DVector& Origin,double Angle, bool bInter, double T0, double T1):axis(Axis),origin(Origin),angle(Angle)
{
    nT0 = T0;
    nT1 = T1;
    bInterpolate = bInter;
}

SScale::SScale(const basegfx::B3DVector& Scale,const basegfx::B3DVector& Origin, bool bInter, double T0, double T1):scale(Scale),origin(Origin)
{
    nT0 = T0;
    nT1 = T1;
    bInterpolate = bInter;
}

RotateAndScaleDepthByWidth::RotateAndScaleDepthByWidth(const basegfx::B3DVector& Axis,const basegfx::B3DVector& Origin,double Angle, bool bInter, double T0, double T1):axis(Axis),origin(Origin),angle(Angle)
{
    nT0 = T0;
    nT1 = T1;
    bInterpolate = bInter;
}


STranslate::STranslate(const basegfx::B3DVector& Vector, bool bInter, double T0, double T1):vector(Vector)
{
    nT0 = T0;
    nT1 = T1;
    bInterpolate = bInter;
}

inline double intervalInter(double t, double T0, double T1)
{
    return ( t - T0 ) / ( T1 - T0 );
}

void STranslate::interpolate(double t,double SlideWidthScale,double SlideHeightScale)
{
    if(t <= nT0)
        return;
    if(!bInterpolate || t > nT1)
        t = nT1;
    t = intervalInter(t,nT0,nT1);
    glTranslated(SlideWidthScale*t*vector.getX(),SlideHeightScale*t*vector.getY(),t*vector.getZ());
}

void SRotate::interpolate(double t,double SlideWidthScale,double SlideHeightScale)
{
    if(t <= nT0)
        return;
    if(!bInterpolate || t > nT1)
        t = nT1;
    t = intervalInter(t,nT0,nT1);
    glTranslated(SlideWidthScale*origin.getX(),SlideHeightScale*origin.getY(),origin.getZ());
    glRotated(t*angle,axis.getX(),axis.getY(),axis.getZ());
    glTranslated(-SlideWidthScale*origin.getX(),-SlideHeightScale*origin.getY(),-origin.getZ());
}

void SScale::interpolate(double t,double SlideWidthScale,double SlideHeightScale)
{
    if(t <= nT0)
        return;
    if(!bInterpolate || t > nT1)
        t = nT1;
    t = intervalInter(t,nT0,nT1);
    glTranslated(SlideWidthScale*origin.getX(),SlideHeightScale*origin.getY(),origin.getZ());
    glScaled((1-t) + t*scale.getX(),(1-t) + t*scale.getY(),(1-t) + t*scale.getZ());
    glTranslated(-SlideWidthScale*origin.getX(),-SlideHeightScale*origin.getY(),-origin.getZ());
}

void RotateAndScaleDepthByWidth::interpolate(double t,double SlideWidthScale,double SlideHeightScale)
{
    if(t <= nT0)
        return;
    if(!bInterpolate || t > nT1)
        t = nT1;
    t = intervalInter(t,nT0,nT1);
    glTranslated(SlideWidthScale*origin.getX(),SlideHeightScale*origin.getY(),SlideWidthScale*origin.getZ());
    glRotated(t*angle,axis.getX(),axis.getY(),axis.getZ());
    glTranslated(-SlideWidthScale*origin.getX(),-SlideHeightScale*origin.getY(),-SlideWidthScale*origin.getZ());
}

STranslate* STranslate::clone()
{
    return new STranslate(*this);
}
SRotate* SRotate::clone()
{
    return new SRotate(*this);
}

SScale* SScale::clone()
{
    return new SScale(*this);
}

RotateAndScaleDepthByWidth* RotateAndScaleDepthByWidth::clone()
{
    return new RotateAndScaleDepthByWidth(*this);
}

const Primitive& Primitive::operator=(const Primitive& rvalue)
{
    for(unsigned int i( 0 ); i < rvalue.Operations.size(); ++i)
        Operations.push_back(rvalue.Operations[i]->clone());
    for(unsigned int i( 0 ); i < rvalue.Vertices.size(); ++i)//SPEED! use std::copy or something. this is slow.
        Vertices.push_back(rvalue.Vertices[i]);
    for(unsigned int i( 0 ); i < rvalue.TexCoords.size(); ++i)//SPEED! use std::copy or something. this is slow.
        TexCoords.push_back(rvalue.TexCoords[i]);
    for(unsigned int i( 0 ); i < rvalue.Normals.size(); ++i)//SPEED! use std::copy or something. this is slow.
        Normals.push_back(rvalue.Normals[i]);
    return *this;
}

Primitive::Primitive(const Primitive& rvalue)
{
    for(unsigned int i( 0 ); i < rvalue.Operations.size(); ++i)
        Operations.push_back(rvalue.Operations[i]->clone());
    for(unsigned int i( 0 ); i < rvalue.Vertices.size(); ++i)//SPEED! use std::copy or something. this is slow.
        Vertices.push_back(rvalue.Vertices[i]);
    for(unsigned int i( 0 ); i < rvalue.TexCoords.size(); ++i)//SPEED! use std::copy or something. this is slow.
        TexCoords.push_back(rvalue.TexCoords[i]);
    for(unsigned int i( 0 ); i < rvalue.Normals.size(); ++i)//SPEED! use std::copy or something. this is slow.
        Normals.push_back(rvalue.Normals[i]);
}

void Primitive::pushTriangle(const basegfx::B2DVector& SlideLocation0,const basegfx::B2DVector& SlideLocation1,const basegfx::B2DVector& SlideLocation2)
{
    std::vector<basegfx::B3DVector> Verts;
    std::vector<basegfx::B2DVector> Texs;
    Verts.reserve(3);
    Texs.reserve(3);

    Verts.push_back(basegfx::B3DVector( 2*SlideLocation0.getX() - 1, -2*SlideLocation0.getY() + 1 , 0.0 ));
    Verts.push_back(basegfx::B3DVector( 2*SlideLocation1.getX() - 1, -2*SlideLocation1.getY() + 1 , 0.0 ));
    Verts.push_back(basegfx::B3DVector( 2*SlideLocation2.getX() - 1, -2*SlideLocation2.getY() + 1 , 0.0 ));

    //figure out if they're facing the correct way, and make them face the correct way.
    basegfx::B3DVector Normal( cross( Verts[0] - Verts[1] , Verts[1] - Verts[2] ) );
    if(Normal.getZ() >= 0.0)//if the normal is facing us
    {
        Texs.push_back(SlideLocation0);
        Texs.push_back(SlideLocation1);
        Texs.push_back(SlideLocation2);
    }
    else // if the normal is facing away from us, make it face us
    {
        Texs.push_back(SlideLocation0);
        Texs.push_back(SlideLocation2);
        Texs.push_back(SlideLocation1);
        Verts.clear();
        Verts.push_back(basegfx::B3DVector( 2*SlideLocation0.getX() - 1, -2*SlideLocation0.getY() + 1 , 0.0 ));
        Verts.push_back(basegfx::B3DVector( 2*SlideLocation2.getX() - 1, -2*SlideLocation2.getY() + 1 , 0.0 ));
        Verts.push_back(basegfx::B3DVector( 2*SlideLocation1.getX() - 1, -2*SlideLocation1.getY() + 1 , 0.0 ));
    }

    Vertices.push_back(Verts[0]);
    Vertices.push_back(Verts[1]);
    Vertices.push_back(Verts[2]);

    TexCoords.push_back(Texs[0]);
    TexCoords.push_back(Texs[1]);
    TexCoords.push_back(Texs[2]);

    Normals.push_back(basegfx::B3DVector(0,0,1));//all normals always face the screen when untransformed.
    Normals.push_back(basegfx::B3DVector(0,0,1));//all normals always face the screen when untransformed.
    Normals.push_back(basegfx::B3DVector(0,0,1));//all normals always face the screen when untransformed.
}

