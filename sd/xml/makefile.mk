PRJ=..
PRJNAME=sd

.INCLUDE :  settings.mk

.IF "$(ENABLE_OPENGL)" != "TRUE"
@all:
    @echo "Building without OpenGL Transitions..."
    @test -f transitions-ogl.xml && mv transitions-ogl.xml transitions-ogl || true
.ELSE
@all:
    @test ! -f transitions-ogl.xml && mv transitions-ogl transitions-ogl.xml || true
.ENDIF
