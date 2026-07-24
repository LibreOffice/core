m4_changequote([,])m4_dnl
m4_dnl# foreachq(x, `item_1, item_2, ..., item_n', stmt)
m4_dnl# quoted list, alternate improved version
m4_define([m4_foreachq],[m4_ifelse([$2],[],[],[m4_pushdef([$1])_$0([$1],[$3],[],$2)m4_popdef([$1])])])m4_dnl
m4_define([_m4_foreachq],[m4_ifelse([$#],[3],[],[m4_define([$1],[$4])$2[]$0([$1],[$2],m4_shift(m4_shift(m4_shift($@))))])])m4_dnl
m4_define([m4_trim],[m4_patsubst([$1],[^. ?\(.*\) .$])])m4_dnl
m4_dnl
// This comment required to work around preprocessor bugs
m4_dnl node_modules
m4_foreachq([fileNode],[NODE_MODULES_JS],[
m4_syscmd([cat ]fileNode)
])
m4_dnl bundled tile utilities
m4_syscmd([cat ]TILEUTILS_JS)
m4_dnl bundled worker
m4_syscmd([cat ]WORKER_JS)
