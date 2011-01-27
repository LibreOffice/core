# Write string to temporary file by chopping into pieces that
# fit the commandline
# parameters: filename, maxitems (for one write), string
# returns: filename
define var2file
$(strip $(1)
$(eval gb_var2file_helpervar := $$(shell printf "%s" "" > $(1) ))\
$(foreach item,$(3),$(eval gb_var2file_curblock += $(item)
    ifeq ($$(words $$(gb_var2file_curblock)),$(2)) 
        gb_var2file_helpervar := $$(shell printf "%s" "$$(gb_var2file_curblock)" >> $(1) )
        gb_var2file_curblock :=
    endif
    ))\
    $(eval gb_var2file_helpervar := $(shell echo " "$(gb_var2file_curblock) >> $(1) )
        gb_var2file_curblock :=
    ))
endef

#define var2file
#$(strip $(1))\
#$(eval 
#gb_Helper_STARTS := $$(shell seq 1 $(2) $$(words $(3)))
#gb_Helper_ENDS := $$(shell seq $(2) $(2) $$(words $(3))) $$(words $(3))
#gb_Helper_RUNS := $$(shell seq $$(words $$(gb_Helper_STARTS)))
#$$(shell printf "" > $(1))
#$$(foreach bufferrun,$$(gb_Helper_RUNS),\
#	$$(shell printf "%s" \
#		"$$(if $$(filter-out 1,$$(bufferrun)), )$$(wordlist \
#			$$(word $$(bufferrun),$$(gb_Helper_STARTS)),\
#			$$(word $$(bufferrun),$$(gb_Helper_ENDS)),$(3))" >> $(1)))
#$$(shell printf "\n" >> $(1))
#)
#endef

define uniqname
$(shell mktemp | tr -d '\n')
endef

