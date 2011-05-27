define writerfilter_debug_flags
$(if $(filter-out 0 1,$(strip $(gb_DEBUGLEVEL))),\
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
