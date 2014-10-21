//
//  saldata.hxx
//
//
//  Created by ptyl on 10/20/14.
//
//

#ifndef INCLUDED_VCL_INC_OPENGL_SALDATA_H
#define INCLUDED_VCL_INC_OPENGL_SALDATA_H

#include <generic/gendata.hxx>

class OpenGLSalData : public SalGenericData
{
public:
    OpenGLSalData( SalInstance *pInstance ) : SalGenericData( SAL_DATA_OPEN_GL, pInstance ) {}
    virtual void ErrorTrapPush() {}
    virtual bool ErrorTrapPop( bool ) { return false; }
};
#endif /* INCLUDED_VCL_INC_OPENGL_SALDATA_H */
