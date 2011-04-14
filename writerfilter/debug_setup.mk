define writerfilter_debug_flags
$(if $(filter $(strip $(gb_DEBUG)),01),\
    -DDEBUG_DOMAINMAPPER \
    -DDEBUG_ELEMENT \
    -DDEBUG_RESOLVE \
    -DDEBUG_CONTEXT_STACK \
    -DDEBUG_ATTRIBUTES \
    -DDEBUG_PROPERTIES \
    -DDEBUG_CONTEXT_HANDLER \
    -DDEBUG_IMPORT \
)
endef
