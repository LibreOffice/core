char* publicFunc = 
 "int isNan(double a) { return a != a; }\n"
 "double fsum(double a, double b) { return isNan(a)?b:a+b; }\n"
 "double fsub(double a, double b) { return a-b; }\n"
 "double fdiv(double a, double b) { return a/b; }\n"
 ;