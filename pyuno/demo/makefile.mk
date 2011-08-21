PRJNAME=pyuno
PRJ=..

.INCLUDE : settings.mk
.INCLUDE : pyversion.mk

ROOT=$(MISC)/pyuno-doc

FILES=\
    $(ROOT)/python-bridge.html \
    $(ROOT)/customized_setup.png \
    $(ROOT)/mode_component.png \
    $(ROOT)/mode_ipc.png \
    $(ROOT)/modes.sxd \
    $(ROOT)/optional_components.png \
    $(ROOT)/samples/swriter.py \
    $(ROOT)/samples/swritercomp.py \
    $(ROOT)/samples/ooextract.py \
    $(ROOT)/samples/biblioaccess.py \
    $(ROOT)/samples/swritercompclient.py \
    $(ROOT)/samples/hello_world_pyuno.zip


$(MISC)/pyuno-doc.zip : dirs $(FILES)
	-rm -f $@
	cd $(MISC) && zip -r pyuno-doc.zip pyuno-doc

dirs .PHONY :
	-mkdir $(ROOT)
	-mkdir $(ROOT)/samples

$(ROOT)/samples/hello_world_pyuno.zip : hello_world_comp.py Addons.xcu
	-rm -f $@
	zip $@ hello_world_comp.py Addons.xcu

$(ROOT)/samples/% : %
	-rm -f $@
	$(COPY) $? $@

$(ROOT)/% : ../doc/%
	-rm -f $@
	$(COPY) $? $@
