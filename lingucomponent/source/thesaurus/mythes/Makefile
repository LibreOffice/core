
CXX=g++

CXXFLAGS= -O2 -Wall -ansi -pedantic -I.

LDFLAGS=-L. -lmythes

LIBS=libmythes.a

AR=ar rc
RANLIB=ranlib

OBJS = mythes.o 

all: example

libmythes.a: $(OBJS)
	$(AR) $@ $(OBJS)
	-@ ($(RANLIB) $@ || true) >/dev/null 2>&1

example: example.o $(LIBS)
	$(CXX) $(CXXFLAGS) -o $@ example.o $(LDFLAGS)

%.o: %.cxx 
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f *.o *~ example libthes.a

distclean:	clean

depend:
	makedepend -- $(CXXFLAGS) -- *.[ch]xx

# DO NOT DELETE THIS LINE -- make depend depends on it.

mythes.o: mythes.hxx
example.o: mythes.hxx 

